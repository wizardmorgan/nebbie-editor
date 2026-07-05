#pragma once

#include "io.hpp"
#include "world.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace nebbie {

enum class OverlayExportKind {
    all,
    rooms,
    objects,
    mobiles,
    zone_resets,
};

struct OverlayImportReport {
    int rooms = 0;
    int objects = 0;
    int mobiles = 0;
    int zone_resets = 0;
    std::vector<std::string> warnings;
};

void save_room_overlay(const Room& room, const World& world, const std::filesystem::path& path);
void load_room_overlay(World& world, long vnum, const std::filesystem::path& path);

void save_object_overlay(const GameObject& obj, const std::filesystem::path& path);
void load_object_overlay(World& world, long vnum, const std::filesystem::path& path);

void save_mobile_overlay(const Mobile& mob, const std::filesystem::path& path);
void load_mobile_overlay(World& world, long vnum, const std::filesystem::path& path);

void save_zone_reset_overlay(const Zone& zone, const std::filesystem::path& path);
void load_zone_reset_overlay(World& world, int zone_num, const std::filesystem::path& path);

OverlayImportReport export_myst_to_overlays(const World& world,
                                            const std::filesystem::path& lib_root,
                                            OverlayExportKind kind = OverlayExportKind::all,
                                            ProgressCallback progress = {});

OverlayImportReport apply_overlays(World& world,
                                   const std::filesystem::path& lib_root,
                                   ProgressCallback progress = {});

} // namespace nebbie
