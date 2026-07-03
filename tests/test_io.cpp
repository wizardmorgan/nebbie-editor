#include "nebbie/io.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: nebbie-core-tests <lib-directory>\n";
            return 1;
        }

        nebbie::World world;
        nebbie::load_lib(world, argv[1]);

        if (world.zones.empty()) {
            throw std::runtime_error("expected at least one zone");
        }
        if (world.rooms.empty()) {
            throw std::runtime_error("expected at least one room");
        }

        const auto out = std::filesystem::temp_directory_path() / "nebbie-editor-test";
        std::filesystem::create_directories(out);
        nebbie::save_myst_zon(world, out / "myst.zon");
        nebbie::save_myst_wld(world, out / "myst.wld");

        nebbie::World roundtrip;
        nebbie::load_lib(roundtrip, out);
        if (roundtrip.zones.size() != world.zones.size()) {
            throw std::runtime_error("zone roundtrip failed");
        }
        if (roundtrip.rooms.size() != world.rooms.size()) {
            throw std::runtime_error("room roundtrip failed");
        }

        std::cout << "OK\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAILED: " << ex.what() << '\n';
        return 1;
    }
}
