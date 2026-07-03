#include "nebbie/io.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: nebbie-phase4-tests <lib-directory>\n";
            return 1;
        }

        const auto lib_root = std::filesystem::path(argv[1]);
        nebbie::World world;
        nebbie::load_lib(world, lib_root);

        const bool has_dam = std::filesystem::exists(lib_root / "myst.dam");
        const bool has_act = std::filesystem::exists(lib_root / "myst.act");
        const bool has_pos = std::filesystem::exists(lib_root / "myst.pos");
        const bool has_gui = std::filesystem::exists(lib_root / "myst.gui");

        if (!has_dam && !has_act && !has_pos && !has_gui) {
            throw std::runtime_error("no phase-4 files found in lib directory");
        }

        const auto out = std::filesystem::temp_directory_path() / "nebbie-phase4-test";
        std::filesystem::create_directories(out);
        if (has_dam) {
            nebbie::save_myst_dam(world, out / "myst.dam");
        }
        if (has_act) {
            nebbie::save_myst_act(world, out / "myst.act");
        }
        if (has_pos) {
            nebbie::save_myst_pos(world, out / "myst.pos");
        }
        if (has_gui) {
            nebbie::save_myst_gui(world, out / "myst.gui");
        }

        nebbie::World roundtrip;
        nebbie::load_lib(roundtrip, out);
        if (has_dam && roundtrip.damage_messages.size() != world.damage_messages.size()) {
            throw std::runtime_error("damage message roundtrip failed");
        }
        if (has_act && roundtrip.social_messages.size() != world.social_messages.size()) {
            throw std::runtime_error("social message roundtrip failed");
        }
        if (has_pos && roundtrip.pose_entries.size() != world.pose_entries.size()) {
            throw std::runtime_error("pose entry roundtrip failed");
        }
        if (has_gui && roundtrip.guilds.size() != world.guilds.size()) {
            throw std::runtime_error("guild entry roundtrip failed");
        }

        std::cout << "OK\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAILED: " << ex.what() << '\n';
        return 1;
    }
}
