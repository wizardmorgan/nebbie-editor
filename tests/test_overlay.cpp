#include "nebbie/constants.hpp"
#include "nebbie/edit.hpp"
#include "nebbie/io.hpp"
#include "nebbie/overlay_io.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {

void copy_fixture_lib(const std::filesystem::path& fixture_root, const std::filesystem::path& out) {
    std::filesystem::create_directories(out);
    for (const auto& entry : std::filesystem::directory_iterator(fixture_root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        std::filesystem::copy_file(entry.path(), out / entry.path().filename(),
                                   std::filesystem::copy_options::overwrite_existing);
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: nebbie-overlay-tests <lib-directory>\n";
            return 1;
        }

        const auto fixture_root = std::filesystem::path(argv[1]);
        const auto work = std::filesystem::temp_directory_path() / "nebbie-overlay-test";
        if (std::filesystem::exists(work)) {
            std::filesystem::remove_all(work);
        }
        copy_fixture_lib(fixture_root, work);

        nebbie::World world;
        nebbie::load_lib(world, work);

        const auto report = nebbie::export_myst_to_overlays(world, work);
        if (report.rooms < 1 || report.objects < 1 || report.mobiles < 1 || report.zone_resets < 1) {
            throw std::runtime_error("export produced empty overlay set");
        }

        const auto room_path = work / nebbie::OVERLAY_ROOMS_DIR / "3001";
        if (!std::filesystem::exists(room_path)) {
            throw std::runtime_error("missing room overlay file");
        }

        {
            std::ifstream in(room_path);
            std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            const auto marker = "Una piccola stanza di test.~";
            const auto pos = content.find(marker);
            if (pos == std::string::npos) {
                throw std::runtime_error("room overlay missing expected description");
            }
            content.replace(pos, std::strlen(marker), "Descrizione overlay aggiornata.~");
            std::ofstream out(room_path, std::ios::trunc);
            out << content;
        }

        nebbie::World overlay_world;
        nebbie::load_lib(overlay_world, work);
        const nebbie::Room* room = overlay_world.find_room(3001);
        if (!room || room->description != "Descrizione overlay aggiornata.") {
            throw std::runtime_error("room overlay was not applied on load");
        }

        const auto zone_path = work / nebbie::OVERLAY_ZONES_DIR / "1.zon";
        if (!std::filesystem::exists(zone_path)) {
            throw std::runtime_error("missing zone reset overlay file");
        }

        {
            std::ofstream out(zone_path, std::ios::trunc);
            out << "M 0 1 1 3001 1\n";
            out << "S\n";
        }

        nebbie::World zone_world;
        nebbie::load_lib(zone_world, work);
        const nebbie::Zone* zone = nebbie::find_zone(zone_world, 1);
        if (!zone || zone->commands.size() != 1 || zone->commands[0].command != 'M') {
            throw std::runtime_error("zone reset overlay was not applied on load");
        }

        std::cout << "OK\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAILED: " << ex.what() << '\n';
        return 1;
    }
}
