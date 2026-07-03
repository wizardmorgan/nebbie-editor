#pragma once

#include "types.hpp"

#include <map>
#include <optional>
#include <string>

namespace nebbie {

class World {
public:
    std::vector<Zone> zones;
    std::map<long, Room> rooms;

    void clear();

    const Zone* zone_for_vnum(long vnum) const;
    Zone* zone_for_vnum(long vnum);
    Room* find_room(long vnum);
    const Room* find_room(long vnum) const;
};

LibPaths lib_paths_for(const std::string& lib_root);

} // namespace nebbie
