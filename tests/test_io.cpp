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
        if (world.mobiles.empty()) {
            throw std::runtime_error("expected at least one mobile");
        }
        if (world.objects.empty()) {
            throw std::runtime_error("expected at least one object");
        }
        if (world.shops.empty()) {
            throw std::runtime_error("expected at least one shop");
        }
        if (world.special_procs.empty()) {
            throw std::runtime_error("expected at least one special proc");
        }

        const auto lib_root = std::filesystem::path(argv[1]);
        const bool has_dam = std::filesystem::exists(lib_root / "myst.dam");
        const bool has_act = std::filesystem::exists(lib_root / "myst.act");
        const bool has_pos = std::filesystem::exists(lib_root / "myst.pos");
        const bool has_gui = std::filesystem::exists(lib_root / "myst.gui");

        if (has_dam && world.damage_messages.empty()) {
            throw std::runtime_error("expected at least one damage message");
        }
        if (has_act && world.social_messages.empty()) {
            throw std::runtime_error("expected at least one social message");
        }
        if (has_pos && world.pose_entries.empty()) {
            throw std::runtime_error("expected at least one pose entry");
        }
        if (has_gui && world.guilds.empty()) {
            throw std::runtime_error("expected at least one guild entry");
        }

        const auto out = std::filesystem::temp_directory_path() / "nebbie-editor-test";
        std::filesystem::create_directories(out);
        nebbie::save_myst_zon(world, out / "myst.zon");
        nebbie::save_myst_wld(world, out / "myst.wld");
        nebbie::save_myst_mob(world, out / "myst.mob");
        nebbie::save_myst_obj(world, out / "myst.obj");
        nebbie::save_myst_shp(world, out / "myst.shp");
        nebbie::save_myst_spe(world, out / "myst.spe");
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
        if (roundtrip.zones.size() != world.zones.size()) {
            throw std::runtime_error("zone roundtrip failed");
        }
        if (roundtrip.rooms.size() != world.rooms.size()) {
            throw std::runtime_error("room roundtrip failed");
        }
        if (roundtrip.mobiles.size() != world.mobiles.size()) {
            throw std::runtime_error("mobile roundtrip failed");
        }
        if (roundtrip.objects.size() != world.objects.size()) {
            throw std::runtime_error("object roundtrip failed");
        }
        if (roundtrip.shops.size() != world.shops.size()) {
            throw std::runtime_error("shop roundtrip failed");
        }
        if (roundtrip.special_procs.size() != world.special_procs.size()) {
            throw std::runtime_error("special proc roundtrip failed");
        }
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
