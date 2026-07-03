#include "nebbie/world.hpp"

#include "nebbie/constants.hpp"

#include <filesystem>

namespace nebbie {

void World::clear() {
    zones.clear();
    rooms.clear();
    mobiles.clear();
    objects.clear();
    shops.clear();
    special_procs.clear();
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

Mobile* World::find_mobile(long vnum) {
    auto it = mobiles.find(vnum);
    return it == mobiles.end() ? nullptr : &it->second;
}

const Mobile* World::find_mobile(long vnum) const {
    auto it = mobiles.find(vnum);
    return it == mobiles.end() ? nullptr : &it->second;
}

GameObject* World::find_object(long vnum) {
    auto it = objects.find(vnum);
    return it == objects.end() ? nullptr : &it->second;
}

const GameObject* World::find_object(long vnum) const {
    auto it = objects.find(vnum);
    return it == objects.end() ? nullptr : &it->second;
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
