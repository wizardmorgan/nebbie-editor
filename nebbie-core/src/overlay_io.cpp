#include "nebbie/overlay_io.hpp"

#include "nebbie/constants.hpp"
#include "nebbie/edit.hpp"

#include <cctype>
#include <filesystem>

namespace nebbie {

namespace {

std::filesystem::path overlay_root(const std::filesystem::path& lib_root, const char* subdir) {
    return resolve_lib_directory(lib_root) / subdir;
}

bool parse_vnum_filename(const std::filesystem::path& path, long& vnum_out) {
    const std::string stem = path.filename().string();
    if (stem.empty() || !std::isdigit(static_cast<unsigned char>(stem[0]))) {
        return false;
    }
    try {
        vnum_out = std::stol(stem);
        return vnum_out > 0;
    } catch (const std::exception&) {
        return false;
    }
}

bool parse_zone_overlay_filename(const std::filesystem::path& path, int& zone_num_out) {
    std::string stem = path.stem().string();
    if (stem.empty() && path.has_filename()) {
        stem = path.filename().string();
    }
    const auto dot = stem.find('.');
    if (dot != std::string::npos) {
        stem = stem.substr(0, dot);
    }
    if (stem.empty() || !std::isdigit(static_cast<unsigned char>(stem[0]))) {
        return false;
    }
    try {
        zone_num_out = std::stoi(stem);
        return zone_num_out > 0;
    } catch (const std::exception&) {
        return false;
    }
}

void export_rooms(const World& world, const std::filesystem::path& root, OverlayImportReport& report,
                  ProgressCallback progress) {
    for (const auto& [vnum, room] : world.rooms) {
        (void)vnum;
        const auto path = root / std::to_string(room.vnum);
        if (progress) {
            progress("Export room overlay " + path.string());
        }
        save_room_overlay(room, world, path);
        ++report.rooms;
    }
}

void export_objects(const World& world, const std::filesystem::path& root, OverlayImportReport& report,
                    ProgressCallback progress) {
    for (const auto& [vnum, obj] : world.objects) {
        (void)vnum;
        const auto path = root / std::to_string(obj.vnum);
        if (progress) {
            progress("Export object overlay " + path.string());
        }
        save_object_overlay(obj, path);
        ++report.objects;
    }
}

void export_mobiles(const World& world, const std::filesystem::path& root, OverlayImportReport& report,
                    ProgressCallback progress) {
    report.warnings.push_back(
        "mobiles/ overlay export is experimental: NebbieArcane read_mobile() may not load mobiles-only files yet");
    for (const auto& [vnum, mob] : world.mobiles) {
        (void)vnum;
        const auto path = root / std::to_string(mob.vnum);
        if (progress) {
            progress("Export mobile overlay " + path.string());
        }
        save_mobile_overlay(mob, path);
        ++report.mobiles;
    }
}

void export_zone_resets(const World& world, const std::filesystem::path& root, OverlayImportReport& report,
                        ProgressCallback progress) {
    for (const auto& zone : world.zones) {
        const auto path = root / (std::to_string(zone.num) + ".zon");
        if (progress) {
            progress("Export zone reset overlay " + path.string());
        }
        save_zone_reset_overlay(zone, path);
        ++report.zone_resets;
    }
}

void apply_room_overlays(World& world, const std::filesystem::path& root, OverlayImportReport& report,
                         ProgressCallback progress) {
    if (!std::filesystem::exists(root)) {
        return;
    }
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        long vnum = 0;
        if (!parse_vnum_filename(entry.path(), vnum)) {
            continue;
        }
        if (progress) {
            progress("Apply room overlay " + entry.path().string());
        }
        load_room_overlay(world, vnum, entry.path());
        ++report.rooms;
    }
}

void apply_object_overlays(World& world, const std::filesystem::path& root, OverlayImportReport& report,
                           ProgressCallback progress) {
    if (!std::filesystem::exists(root)) {
        return;
    }
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        long vnum = 0;
        if (!parse_vnum_filename(entry.path(), vnum)) {
            continue;
        }
        if (progress) {
            progress("Apply object overlay " + entry.path().string());
        }
        load_object_overlay(world, vnum, entry.path());
        ++report.objects;
    }
}

void apply_mobile_overlays(World& world, const std::filesystem::path& root, OverlayImportReport& report,
                           ProgressCallback progress) {
    if (!std::filesystem::exists(root)) {
        return;
    }
    report.warnings.push_back(
        "mobiles/ overlay apply is experimental until server read_mobile() supports overlay-only files");
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        long vnum = 0;
        if (!parse_vnum_filename(entry.path(), vnum)) {
            continue;
        }
        if (progress) {
            progress("Apply mobile overlay " + entry.path().string());
        }
        load_mobile_overlay(world, vnum, entry.path());
        ++report.mobiles;
    }
}

void apply_zone_reset_overlays(World& world, const std::filesystem::path& root, OverlayImportReport& report,
                               ProgressCallback progress) {
    if (!std::filesystem::exists(root)) {
        return;
    }
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        int zone_num = 0;
        if (!parse_zone_overlay_filename(entry.path(), zone_num)) {
            continue;
        }
        if (progress) {
            progress("Apply zone reset overlay " + entry.path().string());
        }
        load_zone_reset_overlay(world, zone_num, entry.path());
        ++report.zone_resets;
    }
}

} // namespace

OverlayImportReport export_myst_to_overlays(const World& world,
                                            const std::filesystem::path& lib_root,
                                            const OverlayExportKind kind,
                                            ProgressCallback progress) {
    OverlayImportReport report;
    const std::filesystem::path root = resolve_lib_directory(lib_root);

    if (kind == OverlayExportKind::all || kind == OverlayExportKind::rooms) {
        export_rooms(world, overlay_root(root, OVERLAY_ROOMS_DIR), report, progress);
    }
    if (kind == OverlayExportKind::all || kind == OverlayExportKind::objects) {
        export_objects(world, overlay_root(root, OVERLAY_OBJECTS_DIR), report, progress);
    }
    if (kind == OverlayExportKind::all || kind == OverlayExportKind::mobiles) {
        export_mobiles(world, overlay_root(root, OVERLAY_MOBILES_DIR), report, progress);
    }
    if (kind == OverlayExportKind::all || kind == OverlayExportKind::zone_resets) {
        export_zone_resets(world, overlay_root(root, OVERLAY_ZONES_DIR), report, progress);
    }

    return report;
}

OverlayImportReport apply_overlays(World& world,
                                   const std::filesystem::path& lib_root,
                                   ProgressCallback progress) {
    OverlayImportReport report;
    const std::filesystem::path root = resolve_lib_directory(lib_root);

    apply_zone_reset_overlays(world, overlay_root(root, OVERLAY_ZONES_DIR), report, progress);
    apply_room_overlays(world, overlay_root(root, OVERLAY_ROOMS_DIR), report, progress);
    apply_mobile_overlays(world, overlay_root(root, OVERLAY_MOBILES_DIR), report, progress);
    apply_object_overlays(world, overlay_root(root, OVERLAY_OBJECTS_DIR), report, progress);

    return report;
}

} // namespace nebbie
