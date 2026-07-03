#include "nebbie/io.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        const auto mob_path = (argc >= 2)
                                  ? std::filesystem::path(argv[1])
                                  : std::filesystem::path("vendor/nebbietest/myst.mob");

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
        if (!mob_s || mob_s->mobtype != 'S' || mob_s->hit_dice.empty() || mob_s->dam_dice.empty()) {
            throw std::runtime_error("type S mob #1 not parsed correctly");
        }
        if (!mob_a || mob_a->mobtype != 'A' || mob_a->hit_bonus != 200 || mob_a->dam_dice != "2d6+8") {
            throw std::runtime_error("type A mob #3 not parsed correctly");
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
