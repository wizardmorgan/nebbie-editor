#include "nebbie/io.hpp"
#include "nebbie/world_index.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: nebbie-world-index-tests <lib-directory> [output.json]\n";
            return 1;
        }

        nebbie::World world;
        nebbie::load_lib(world, argv[1]);
        const nebbie::WorldIndex built = nebbie::build_world_index(world, "test-fixture");
        const std::string json = nebbie::world_index_to_json(built);
        const auto parsed = nebbie::world_index_from_json(json);
        if (!parsed) {
            throw std::runtime_error("roundtrip JSON parse failed");
        }
        if (parsed->zones.size() != built.zones.size()) {
            throw std::runtime_error("zone count mismatch after roundtrip");
        }
        if (parsed->mob_vnums.size() != built.mob_vnums.size()) {
            throw std::runtime_error("mob count mismatch after roundtrip");
        }
        if (built.zones.front().bottom < 0 || built.zones.front().top < built.zones.front().bottom) {
            throw std::runtime_error("invalid zone bottom/top in world index");
        }

        const auto suggested = nebbie::suggest_room_vnum_in_zone(built, built.zones.front().zone_num);
        if (!suggested) {
            throw std::runtime_error("expected a suggested room vnum");
        }
        if (nebbie::room_vnum_taken(built, *suggested)) {
            throw std::runtime_error("suggested vnum should not be taken");
        }

        nebbie::WorldIndexReservation valid;
        valid.builder = "Test";
        valid.zone_num = built.zones.front().zone_num;
        valid.kind = "room";
        valid.start_vnum = built.zones.front().bottom;
        valid.end_vnum = built.zones.front().bottom;
        std::string error;
        if (!nebbie::validate_reservation(built, valid, &error)) {
            throw std::runtime_error("valid reservation rejected: " + error);
        }

        nebbie::WorldIndexReservation missing_end;
        missing_end.builder = "Test";
        missing_end.zone_num = built.zones.front().zone_num;
        missing_end.kind = "mob";
        missing_end.start_vnum = 1;
        missing_end.end_vnum = 0;
        if (nebbie::validate_reservation(built, missing_end, &error)) {
            throw std::runtime_error("reservation without end_vnum should fail");
        }

        const std::string api_json = R"({"status":"OK","reservations":[{"builder":"Test","zone_num":1,"kind":"room","start_vnum":99990,"end_vnum":99999,"note":"","expires_at":"","status":"active"}]})";
        const auto reservations = nebbie::coordinator_reservations_from_json(api_json);
        if (!reservations || reservations->size() != 1) {
            throw std::runtime_error("coordinator reservations parse failed");
        }
        nebbie::WorldIndex with_reservations = built;
        nebbie::merge_reservations(with_reservations, *reservations);
        if (!nebbie::room_vnum_taken(with_reservations, 99995)) {
            throw std::runtime_error("reserved vnum should be taken");
        }

        if (argc >= 3) {
            std::ofstream out(argv[2]);
            out << json;
        }

        std::cout << "world-index tests ok\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "world-index tests failed: " << ex.what() << '\n';
        return 1;
    }
}
