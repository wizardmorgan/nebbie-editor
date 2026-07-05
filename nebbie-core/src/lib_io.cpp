#include "nebbie/io.hpp"
#include "nebbie/overlay_io.hpp"

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

bool lib_marker_exists(const std::filesystem::path& dir) {
    return std::filesystem::exists(dir / ZONE_FILE) || std::filesystem::exists(dir / MOB_FILE)
           || std::filesystem::exists(dir / WORLD_FILE) || std::filesystem::exists(dir / OBJ_FILE);
}

} // namespace

std::filesystem::path resolve_lib_directory(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::path candidate = path;
    if (std::filesystem::is_regular_file(candidate, ec)) {
        const std::string name = candidate.filename().string();
        if (name == ZONE_FILE || name == MOB_FILE || name == WORLD_FILE || name == OBJ_FILE) {
            candidate = candidate.parent_path();
        }
    }

    if (lib_marker_exists(candidate)) {
        return candidate;
    }

    const auto lib_subdir = candidate / "lib";
    if (lib_marker_exists(lib_subdir)) {
        return lib_subdir;
    }

    return candidate;
}

void load_lib(World& world, const std::filesystem::path& lib_root, ProgressCallback progress) {
    LibContext context;
    load_lib(world, lib_root, context, progress);
}

void load_lib(World& world,
              const std::filesystem::path& lib_root,
              LibContext& context,
              ProgressCallback progress) {
    world.clear();
    context = {};

    const std::filesystem::path resolved = resolve_lib_directory(lib_root);
    context.root = resolved;

    load_if_exists(resolved / ZONE_FILE, context.has_zon, [&]() {
        load_myst_zon(world, resolved / ZONE_FILE, progress);
    });
    load_if_exists(resolved / WORLD_FILE, context.has_wld, [&]() {
        load_myst_wld(world, resolved / WORLD_FILE, progress);
    });
    load_if_exists(resolved / MOB_FILE, context.has_mob, [&]() {
        load_myst_mob(world, resolved / MOB_FILE, progress);
    });
    load_if_exists(resolved / OBJ_FILE, context.has_obj, [&]() {
        load_myst_obj(world, resolved / OBJ_FILE, progress);
    });
    load_if_exists(resolved / SHOP_FILE, context.has_shp, [&]() {
        load_myst_shp(world, resolved / SHOP_FILE, progress);
    });
    load_if_exists(resolved / SPECIAL_FILE, context.has_spe, [&]() {
        load_myst_spe(world, resolved / SPECIAL_FILE, progress);
    });
    load_if_exists(resolved / DAMAGE_FILE, context.has_dam, [&]() {
        load_myst_dam(world, resolved / DAMAGE_FILE, progress);
    });
    load_if_exists(resolved / SOCIAL_FILE, context.has_act, [&]() {
        load_myst_act(world, resolved / SOCIAL_FILE, progress);
    });
    load_if_exists(resolved / POSE_FILE, context.has_pos, [&]() {
        load_myst_pos(world, resolved / POSE_FILE, progress);
    });
    load_if_exists(resolved / GUILD_FILE, context.has_gui, [&]() {
        load_myst_gui(world, resolved / GUILD_FILE, progress);
    });

    const OverlayImportReport overlay_report = apply_overlays(world, resolved, progress);
    if (progress && (overlay_report.rooms > 0 || overlay_report.objects > 0 || overlay_report.mobiles > 0
                     || overlay_report.zone_resets > 0)) {
        progress("Applied overlays: rooms=" + std::to_string(overlay_report.rooms) + " objects="
                 + std::to_string(overlay_report.objects) + " mobiles="
                 + std::to_string(overlay_report.mobiles) + " zone_resets="
                 + std::to_string(overlay_report.zone_resets));
    }
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
