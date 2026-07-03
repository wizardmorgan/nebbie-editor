#include "nebbie/io.hpp"

#include "nebbie/constants.hpp"

#include <filesystem>

namespace nebbie {

void load_lib(World& world, const std::filesystem::path& lib_root, ProgressCallback progress) {
    world.clear();

    const auto zon = lib_root / ZONE_FILE;
    const auto wld = lib_root / WORLD_FILE;

    if (std::filesystem::exists(zon)) {
        load_myst_zon(world, zon, progress);
    }

    if (std::filesystem::exists(wld)) {
        load_myst_wld(world, wld, progress);
    }

    const auto mob = lib_root / MOB_FILE;
    const auto obj = lib_root / OBJ_FILE;

    if (std::filesystem::exists(mob)) {
        load_myst_mob(world, mob, progress);
    }

    if (std::filesystem::exists(obj)) {
        load_myst_obj(world, obj, progress);
    }
}

} // namespace nebbie
