#include "nebbie/io.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
    try {
        std::filesystem::path mob_path = "vendor/nebbietest/myst.mob";
        std::filesystem::path lib_path;

        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--lib" && i + 1 < argc) {
                lib_path = argv[++i];
            } else if (arg.rfind('-', 0) != 0) {
                mob_path = arg;
            }
        }

        if (!lib_path.empty()) {
            if (!std::filesystem::exists(lib_path / "myst.mob")) {
                std::cerr << "SKIP: myst.mob not found in " << lib_path << '\n';
                return 0;
            }
            nebbie::World world;
            nebbie::load_lib(world, lib_path);
            if (world.mobiles.empty()) {
                throw std::runtime_error("expected mobiles from nebbietest mudroot/lib");
            }
            const nebbie::Mobile* druid = world.find_mobile(2111);
            if (druid && (druid->hitroll != 15 || druid->dam_dice != "2d6+4")) {
                throw std::runtime_error("mob #2111 THAC0 annotation not parsed correctly in lib load");
            }
            std::cout << "OK lib (" << world.mobiles.size() << " mobiles, "
                      << world.rooms.size() << " rooms)\n";
            return 0;
        }

        if (!std::filesystem::exists(mob_path)) {
            std::cerr << "SKIP: mob file not found: " << mob_path << '\n';
            return 0;
        }

        nebbie::World world;
        nebbie::load_myst_mob(world, mob_path);
        if (world.mobiles.size() < 10) {
            throw std::runtime_error("expected many mobiles from nebbietest myst.mob");
        }

        const nebbie::Mobile* mob_s = world.find_mobile(1);
        const nebbie::Mobile* mob_a = world.find_mobile(3);
        const nebbie::Mobile* mob_annot = world.find_mobile(2111);
        const nebbie::Mobile* mob_sounds = world.find_mobile(2112);
        if (!mob_s || mob_s->mobtype != 'S' || mob_s->hit_dice.empty() || mob_s->dam_dice.empty()) {
            throw std::runtime_error("type S mob #1 not parsed correctly");
        }
        if (!mob_a || mob_a->mobtype != 'A' || mob_a->hit_bonus != 200 || mob_a->dam_dice != "2d6+8") {
            throw std::runtime_error("type A mob #3 not parsed correctly");
        }
        if (mob_annot) {
            if (mob_annot->level != 11 || mob_annot->hitroll != 15 || mob_annot->hit_bonus != 8
                || mob_annot->dam_dice != "2d6+4") {
                throw std::runtime_error("type A mob #2111 THAC0 annotation not parsed correctly");
            }
        }
        if (mob_sounds) {
            if (mob_sounds->mobtype != 'A' || mob_sounds->sounds.find("chanting") == std::string::npos
                || mob_sounds->extra_sound_strings.size() != 1
                || mob_sounds->extra_sound_strings[0].find("Whispers") == std::string::npos) {
                throw std::runtime_error("type A mob #2112 trailing sound strings not parsed correctly");
            }
        }

        const nebbie::Mobile* mob_2113 = world.find_mobile(2113);
        if (mob_2113) {
            if (mob_2113->mult_att != 1 || mob_2113->hitroll != 7 || mob_2113->hit_bonus != 40
                || mob_2113->meta_immune != 2048) {
                throw std::runtime_error("mob #2113 type line or = annotations not parsed correctly");
            }
        }

        const auto fixture = std::filesystem::path("tests/fixtures/myst.mob");
        if (std::filesystem::exists(fixture)) {
            nebbie::World fixture_world;
            nebbie::load_myst_mob(fixture_world, fixture);
            if (fixture_world.mobiles.size() != 5 || !fixture_world.find_mobile(2111)) {
                throw std::runtime_error("fixture myst.mob with # comment line did not load");
            }
        }

        const nebbie::Mobile* vendor_ron = world.find_mobile(1717);
        if (vendor_ron && vendor_ron->mobtype == 'L') {
            if (vendor_ron->sounds.find("Compai") == std::string::npos
                || vendor_ron->distant_sounds.find("muffled laughter") == std::string::npos) {
                throw std::runtime_error("type L mob #1717 sound strings not parsed correctly");
            }
        }

        const auto out = std::filesystem::path("build/nebbietest-mob-rt");
        std::filesystem::create_directories(out);
        nebbie::save_myst_mob(world, out / "myst.mob");

        nebbie::World roundtrip;
        nebbie::load_myst_mob(roundtrip, out / "myst.mob");
        if (roundtrip.mobiles.size() != world.mobiles.size()) {
            throw std::runtime_error("mob count mismatch after roundtrip");
        }

        const nebbie::Mobile* rt_a = roundtrip.find_mobile(3);
        if (!rt_a || rt_a->hit_bonus != 200 || rt_a->dam_dice != "2d6+8") {
            throw std::runtime_error("type A mob roundtrip failed");
        }

        std::cout << "OK (" << world.mobiles.size() << " mobiles)\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAILED: " << ex.what() << '\n';
        return 1;
    }
}
