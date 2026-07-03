#include "nebbie/edit.hpp"
#include "nebbie/io.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: nebbie-reset-tests <source-lib> <work-lib>\n";
            return 1;
        }

        const auto source = std::filesystem::path(argv[1]);
        const auto work = std::filesystem::path(argv[2]);

        std::error_code ec;
        std::filesystem::remove_all(work, ec);
        std::filesystem::create_directories(work);
        for (const auto& entry : std::filesystem::directory_iterator(source)) {
            if (entry.is_regular_file()) {
                std::filesystem::copy_file(entry.path(), work / entry.path().filename(),
                                           std::filesystem::copy_options::overwrite_existing);
            }
        }

        nebbie::World world;
        nebbie::LibContext context;
        nebbie::load_lib(world, work, context);
        if (world.zones.empty()) {
            throw std::runtime_error("expected at least one zone");
        }

        const nebbie::Zone& zone = world.zones.front();
        const std::size_t before_count = zone.commands.size();
        nebbie::ResetCommand mob_reset = nebbie::default_zone_reset('M', zone, world);
        mob_reset.arg1 = 1;
        mob_reset.arg3 = 3001;
        mob_reset.arg4 = 2;
        if (!nebbie::add_zone_reset(world, zone.num, mob_reset)) {
            throw std::runtime_error("add_zone_reset failed");
        }

        nebbie::Zone* mutable_zone = nebbie::find_zone(world, zone.num);
        if (!mutable_zone || mutable_zone->commands.size() != before_count + 1) {
            throw std::runtime_error("reset not appended");
        }

        const std::size_t new_index = mutable_zone->commands.size() - 1;
        if (!nebbie::move_zone_reset(world, zone.num, new_index, 0)) {
            throw std::runtime_error("move_zone_reset failed");
        }
        if (!nebbie::remove_zone_reset(world, zone.num, mutable_zone->commands.size() - 1)) {
            throw std::runtime_error("remove_zone_reset failed");
        }

        nebbie::save_lib(world, context);

        nebbie::World reloaded;
        nebbie::load_lib(reloaded, work);
        const nebbie::Zone* reloaded_zone = nebbie::find_zone(reloaded, zone.num);
        if (!reloaded_zone) {
            throw std::runtime_error("zone missing after reload");
        }

        bool found_mob_reset = false;
        for (const auto& cmd : reloaded_zone->commands) {
            if (cmd.command == 'M' && cmd.arg1 == 1 && cmd.arg3 == 3001 && cmd.arg4 == 2) {
                found_mob_reset = true;
                break;
            }
        }
        if (!found_mob_reset) {
            throw std::runtime_error("mob reset not persisted");
        }

        if (nebbie::reset_command_summary(reloaded_zone->commands.front()).empty()) {
            throw std::runtime_error("reset summary empty");
        }

        std::cout << "OK\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAILED: " << ex.what() << '\n';
        return 1;
    }
}
