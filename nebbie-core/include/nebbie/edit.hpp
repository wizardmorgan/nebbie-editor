#pragma once

#include "world.hpp"

#include <map>
#include <string>

namespace nebbie {

struct RoomEdit {
    std::string name;
    std::string description;
    long sector_type = -1;
    long room_flags = -1;
};

struct MobEdit {
    std::string short_descr;
    std::string long_descr;
    std::string description;
    int level = -1;
    long alignment = -999999;
};

struct ObjEdit {
    std::string short_descr;
    std::string description;
    int cost = -1;
    int weight = -1;
};

void apply_room_edit(Room& room, const RoomEdit& edit);
void apply_mob_edit(Mobile& mob, const MobEdit& edit);
void apply_obj_edit(GameObject& obj, const ObjEdit& edit);

bool edit_room(World& world, long vnum, const RoomEdit& edit);
bool edit_mob(World& world, long vnum, const MobEdit& edit);
bool edit_object(World& world, long vnum, const ObjEdit& edit);

RoomEdit room_edit_from_flags(const std::map<std::string, std::string>& flags);
MobEdit mob_edit_from_flags(const std::map<std::string, std::string>& flags);
ObjEdit obj_edit_from_flags(const std::map<std::string, std::string>& flags);

} // namespace nebbie
