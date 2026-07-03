#include "nebbie/world.hpp"

#include "nebbie/constants.hpp"

#include <filesystem>

namespace nebbie {

void World::clear() {
    zones.clear();
    rooms.clear();
}

const Zone* World::zone_for_vnum(long vnum) const {
    for (const auto& zone : zones) {
        if (vnum >= zone.bottom && vnum <= zone.top) {
            return &zone;
        }
    }
    return nullptr;
}

Zone* World::zone_for_vnum(long vnum) {
    for (auto& zone : zones) {
        if (vnum >= zone.bottom && vnum <= zone.top) {
            return &zone;
        }
    }
    return nullptr;
}

Room* World::find_room(long vnum) {
    auto it = rooms.find(vnum);
    return it == rooms.end() ? nullptr : &it->second;
}

const Room* World::find_room(long vnum) const {
    auto it = rooms.find(vnum);
    return it == rooms.end() ? nullptr : &it->second;
}

LibPaths lib_paths_for(const std::string& lib_root) {
    const std::filesystem::path root = lib_root;
    return {
        root.string(),
        (root / ZONE_FILE).string(),
        (root / WORLD_FILE).string(),
        (root / MOB_FILE).string(),
        (root / OBJ_FILE).string(),
        (root / SHOP_FILE).string(),
        (root / SPECIAL_FILE).string(),
        (root / DAMAGE_FILE).string(),
        (root / SOCIAL_FILE).string(),
        (root / POSE_FILE).string(),
        (root / GUILD_FILE).string(),
    };
}

} // namespace nebbie
