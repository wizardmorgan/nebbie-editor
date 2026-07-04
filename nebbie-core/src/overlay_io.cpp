#include "nebbie/io.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>
#include <string>

namespace nebbie {

namespace {

bool is_dot_entry(const std::filesystem::path& name) {
    const std::string stem = name.string();
    return stem == "." || stem == "..";
}

std::optional<int> parse_zone_overlay_index(const std::string& filename) {
    std::string stem = filename;
    if (stem.size() > 4 && stem.substr(stem.size() - 4) == ".zon") {
        stem = stem.substr(0, stem.size() - 4);
    }

    if (stem.empty() || !std::isdigit(static_cast<unsigned char>(stem[0]))) {
        return std::nullopt;
    }

    try {
        const int index = std::stoi(stem);
        if (index <= 0) {
            return std::nullopt;
        }
        return index;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<long> parse_vnum_filename(const std::string& filename) {
    if (filename.empty() || !std::isdigit(static_cast<unsigned char>(filename[0]))) {
        return std::nullopt;
    }

    try {
        const long vnum = std::stol(filename);
        if (vnum <= 0) {
            return std::nullopt;
        }
        return vnum;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

} // namespace

void load_zone_overlays(World& world,
                        const std::filesystem::path& zones_dir,
                        LibContext& context,
                        ProgressCallback progress) {
    std::error_code ec;
    if (!std::filesystem::is_directory(zones_dir, ec)) {
        return;
    }

    context.zone_overlay_indices.clear();

    for (const auto& entry : std::filesystem::directory_iterator(zones_dir, ec)) {
        if (ec || !entry.is_regular_file()) {
            continue;
        }

        const std::string filename = entry.path().filename().string();
        if (is_dot_entry(filename)) {
            continue;
        }

        const auto zone_index = parse_zone_overlay_index(filename);
        if (!zone_index) {
            continue;
        }

        load_zone_overlay_file(world, *zone_index, entry.path(), progress);
        context.zone_overlay_indices.push_back(*zone_index);
    }

    std::sort(context.zone_overlay_indices.begin(), context.zone_overlay_indices.end());
    context.zone_overlay_indices.erase(
        std::unique(context.zone_overlay_indices.begin(), context.zone_overlay_indices.end()),
        context.zone_overlay_indices.end());
}

void load_room_overlays(World& world,
                        const std::filesystem::path& rooms_dir,
                        LibContext& context,
                        ProgressCallback progress) {
    std::error_code ec;
    if (!std::filesystem::is_directory(rooms_dir, ec)) {
        return;
    }

    context.room_overlay_vnums.clear();

    for (const auto& entry : std::filesystem::directory_iterator(rooms_dir, ec)) {
        if (ec || !entry.is_regular_file()) {
            continue;
        }

        const std::string filename = entry.path().filename().string();
        if (is_dot_entry(filename)) {
            continue;
        }

        const auto vnum = parse_vnum_filename(filename);
        if (!vnum) {
            continue;
        }

        load_room_overlay_file(world, *vnum, entry.path(), progress);
        context.room_overlay_vnums.push_back(*vnum);
    }

    std::sort(context.room_overlay_vnums.begin(), context.room_overlay_vnums.end());
    context.room_overlay_vnums.erase(
        std::unique(context.room_overlay_vnums.begin(), context.room_overlay_vnums.end()),
        context.room_overlay_vnums.end());
}

void load_object_overlays(World& world,
                          const std::filesystem::path& objects_dir,
                          LibContext& context,
                          ProgressCallback progress) {
    std::error_code ec;
    if (!std::filesystem::is_directory(objects_dir, ec)) {
        return;
    }

    context.object_overlay_vnums.clear();

    for (const auto& entry : std::filesystem::directory_iterator(objects_dir, ec)) {
        if (ec || !entry.is_regular_file()) {
            continue;
        }

        const std::string filename = entry.path().filename().string();
        if (is_dot_entry(filename)) {
            continue;
        }

        const auto vnum = parse_vnum_filename(filename);
        if (!vnum) {
            continue;
        }

        load_object_overlay_file(world, *vnum, entry.path(), progress);
        context.object_overlay_vnums.push_back(*vnum);
    }

    std::sort(context.object_overlay_vnums.begin(), context.object_overlay_vnums.end());
    context.object_overlay_vnums.erase(
        std::unique(context.object_overlay_vnums.begin(), context.object_overlay_vnums.end()),
        context.object_overlay_vnums.end());
}

} // namespace nebbie
