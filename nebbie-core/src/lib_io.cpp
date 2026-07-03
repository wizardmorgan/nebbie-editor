#include "nebbie/io.hpp"

#include "nebbie/constants.hpp"

#include <filesystem>
#include <functional>

namespace nebbie {

namespace {

void load_if_exists(const std::filesystem::path& path,
                    bool& flag,
                    const std::function<void()>& loader) {
    if (std::filesystem::exists(path)) {
        loader();
        flag = true;
    }
}

} // namespace

void load_lib(World& world, const std::filesystem::path& lib_root, ProgressCallback progress) {
    LibContext context;
    context.root = lib_root;
    load_lib(world, lib_root, context, progress);
}

void load_lib(World& world,
              const std::filesystem::path& lib_root,
              LibContext& context,
              ProgressCallback progress) {
    world.clear();
    context = {};
    context.root = lib_root;

    load_if_exists(lib_root / ZONE_FILE, context.has_zon, [&]() {
        load_myst_zon(world, lib_root / ZONE_FILE, progress);
    });
    load_if_exists(lib_root / WORLD_FILE, context.has_wld, [&]() {
        load_myst_wld(world, lib_root / WORLD_FILE, progress);
    });
    load_if_exists(lib_root / MOB_FILE, context.has_mob, [&]() {
        load_myst_mob(world, lib_root / MOB_FILE, progress);
    });
    load_if_exists(lib_root / OBJ_FILE, context.has_obj, [&]() {
        load_myst_obj(world, lib_root / OBJ_FILE, progress);
    });
    load_if_exists(lib_root / SHOP_FILE, context.has_shp, [&]() {
        load_myst_shp(world, lib_root / SHOP_FILE, progress);
    });
    load_if_exists(lib_root / SPECIAL_FILE, context.has_spe, [&]() {
        load_myst_spe(world, lib_root / SPECIAL_FILE, progress);
    });
    load_if_exists(lib_root / DAMAGE_FILE, context.has_dam, [&]() {
        load_myst_dam(world, lib_root / DAMAGE_FILE, progress);
    });
    load_if_exists(lib_root / SOCIAL_FILE, context.has_act, [&]() {
        load_myst_act(world, lib_root / SOCIAL_FILE, progress);
    });
    load_if_exists(lib_root / POSE_FILE, context.has_pos, [&]() {
        load_myst_pos(world, lib_root / POSE_FILE, progress);
    });
    load_if_exists(lib_root / GUILD_FILE, context.has_gui, [&]() {
        load_myst_gui(world, lib_root / GUILD_FILE, progress);
    });
}

void save_lib(const World& world, const LibContext& context, ProgressCallback progress) {
    const auto& root = context.root;
    if (context.has_zon) {
        save_myst_zon(world, root / ZONE_FILE, progress);
    }
    if (context.has_wld) {
        save_myst_wld(world, root / WORLD_FILE, progress);
    }
    if (context.has_mob) {
        save_myst_mob(world, root / MOB_FILE, progress);
    }
    if (context.has_obj) {
        save_myst_obj(world, root / OBJ_FILE, progress);
    }
    if (context.has_shp) {
        save_myst_shp(world, root / SHOP_FILE, progress);
    }
    if (context.has_spe) {
        save_myst_spe(world, root / SPECIAL_FILE, progress);
    }
    if (context.has_dam) {
        save_myst_dam(world, root / DAMAGE_FILE, progress);
    }
    if (context.has_act) {
        save_myst_act(world, root / SOCIAL_FILE, progress);
    }
    if (context.has_pos) {
        save_myst_pos(world, root / POSE_FILE, progress);
    }
    if (context.has_gui) {
        save_myst_gui(world, root / GUILD_FILE, progress);
    }
}

} // namespace nebbie
