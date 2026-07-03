#include "nebbie/edit.hpp"

#include <stdexcept>

namespace nebbie {

namespace {

long parse_long(const std::string& value, const char* field) {
    try {
        return std::stol(value);
    } catch (const std::exception&) {
        throw std::runtime_error(std::string("invalid value for ") + field + ": " + value);
    }
}

int parse_int(const std::string& value, const char* field) {
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        throw std::runtime_error(std::string("invalid value for ") + field + ": " + value);
    }
}

} // namespace

void apply_room_edit(Room& room, const RoomEdit& edit) {
    if (!edit.name.empty()) {
        room.name = edit.name;
    }
    if (!edit.description.empty()) {
        room.description = edit.description;
    }
    if (edit.sector_type >= 0) {
        room.sector_type = edit.sector_type;
    }
    if (edit.room_flags >= 0) {
        room.room_flags = edit.room_flags;
    }
}

void apply_mob_edit(Mobile& mob, const MobEdit& edit) {
    if (!edit.short_descr.empty()) {
        mob.short_descr = edit.short_descr;
    }
    if (!edit.long_descr.empty()) {
        mob.long_descr = edit.long_descr;
    }
    if (!edit.description.empty()) {
        mob.description = edit.description;
    }
    if (edit.level >= 0) {
        mob.level = edit.level;
    }
    if (edit.alignment > -999999) {
        mob.alignment = edit.alignment;
    }
}

void apply_obj_edit(GameObject& obj, const ObjEdit& edit) {
    if (!edit.short_descr.empty()) {
        obj.short_descr = edit.short_descr;
    }
    if (!edit.description.empty()) {
        obj.description = edit.description;
    }
    if (edit.cost >= 0) {
        obj.cost = edit.cost;
    }
    if (edit.weight >= 0) {
        obj.weight = edit.weight;
    }
}

bool edit_room(World& world, long vnum, const RoomEdit& edit) {
    Room* room = world.find_room(vnum);
    if (!room) {
        return false;
    }
    apply_room_edit(*room, edit);
    return true;
}

bool edit_mob(World& world, long vnum, const MobEdit& edit) {
    Mobile* mob = world.find_mobile(vnum);
    if (!mob) {
        return false;
    }
    apply_mob_edit(*mob, edit);
    return true;
}

bool edit_object(World& world, long vnum, const ObjEdit& edit) {
    GameObject* obj = world.find_object(vnum);
    if (!obj) {
        return false;
    }
    apply_obj_edit(*obj, edit);
    return true;
}

RoomEdit room_edit_from_flags(const std::map<std::string, std::string>& flags) {
    RoomEdit edit;
    if (auto it = flags.find("name"); it != flags.end()) {
        edit.name = it->second;
    }
    if (auto it = flags.find("desc"); it != flags.end()) {
        edit.description = it->second;
    }
    if (auto it = flags.find("sector"); it != flags.end()) {
        edit.sector_type = parse_long(it->second, "sector");
    }
    if (auto it = flags.find("flags"); it != flags.end()) {
        edit.room_flags = parse_long(it->second, "flags");
    }
    return edit;
}

MobEdit mob_edit_from_flags(const std::map<std::string, std::string>& flags) {
    MobEdit edit;
    if (auto it = flags.find("short"); it != flags.end()) {
        edit.short_descr = it->second;
    }
    if (auto it = flags.find("long"); it != flags.end()) {
        edit.long_descr = it->second;
    }
    if (auto it = flags.find("desc"); it != flags.end()) {
        edit.description = it->second;
    }
    if (auto it = flags.find("level"); it != flags.end()) {
        edit.level = parse_int(it->second, "level");
    }
    if (auto it = flags.find("alignment"); it != flags.end()) {
        edit.alignment = parse_long(it->second, "alignment");
    }
    return edit;
}

ObjEdit obj_edit_from_flags(const std::map<std::string, std::string>& flags) {
    ObjEdit edit;
    if (auto it = flags.find("short"); it != flags.end()) {
        edit.short_descr = it->second;
    }
    if (auto it = flags.find("desc"); it != flags.end()) {
        edit.description = it->second;
    }
    if (auto it = flags.find("cost"); it != flags.end()) {
        edit.cost = parse_int(it->second, "cost");
    }
    if (auto it = flags.find("weight"); it != flags.end()) {
        edit.weight = parse_int(it->second, "weight");
    }
    return edit;
}

} // namespace nebbie
