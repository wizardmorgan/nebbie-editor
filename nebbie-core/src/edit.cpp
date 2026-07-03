#include "nebbie/edit.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace nebbie {

namespace {

constexpr const char* EXIT_NAMES[EXIT_DIR_COUNT] = {
    "nord", "est", "sud", "ovest", "su", "giu",
};

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

template <typename Map>
long max_vnum_in_map(const Map& entities) {
    long max_vnum = 0;
    for (const auto& [vnum, _] : entities) {
        max_vnum = std::max(max_vnum, vnum);
    }
    return max_vnum;
}

long first_zone_bottom(const World& world) {
    if (world.zones.empty()) {
        return 1;
    }
    return world.zones.front().bottom;
}

int zone_index_for_vnum(const World& world, long vnum) {
    if (const Zone* zone = world.zone_for_vnum(vnum)) {
        for (size_t i = 0; i < world.zones.size(); ++i) {
            if (&world.zones[i] == zone) {
                return static_cast<int>(i);
            }
        }
    }
    return -1;
}

Mobile make_default_mobile(long vnum, const MobEdit& edit) {
    Mobile mob;
    mob.vnum = vnum;
    mob.name = "nuovo mob";
    mob.short_descr = edit.short_descr.empty() ? "un nuovo mob" : edit.short_descr;
    mob.long_descr = edit.long_descr.empty() ? "Un nuovo mob e' qui." : edit.long_descr;
    mob.description = edit.description.empty() ? "Non vedi nulla di speciale." : edit.description;
    mob.act = 1;
    mob.affected_by = 0;
    mob.alignment = edit.alignment > -999999 ? edit.alignment : 0;
    mob.mobtype = 'S';
    mob.mult_att = 1;
    mob.level = edit.level >= 0 ? edit.level : 1;
    mob.hitroll = 0;
    mob.ac = 10;
    mob.hit_dice = "1d1+0";
    mob.dam_dice = "1d4+0";
    mob.gold = 0;
    mob.exp = 0;
    mob.position = 8;
    mob.default_pos = 8;
    mob.sex = 0;
    return mob;
}

GameObject make_default_object(long vnum, const ObjEdit& edit) {
    GameObject obj;
    obj.vnum = vnum;
    obj.name = "oggetto nuovo";
    obj.short_descr = edit.short_descr.empty() ? "un oggetto nuovo" : edit.short_descr;
    obj.description = edit.description.empty() ? "Un oggetto nuovo e' qui." : edit.description;
    obj.action_description.clear();
    obj.type_flag = 1;
    obj.extra_flags = 0;
    obj.wear_flags = 1;
    obj.weight = edit.weight >= 0 ? edit.weight : 1;
    obj.cost = edit.cost >= 0 ? edit.cost : 0;
    obj.cost_per_day = 0;
    return obj;
}

} // namespace

const char* exit_direction_name(int direction) {
    if (direction < 0 || direction >= EXIT_DIR_COUNT) {
        return "?";
    }
    return EXIT_NAMES[direction];
}

int exit_direction_from_name(const std::string& name) {
    std::string lowered;
    lowered.reserve(name.size());
    for (char c : name) {
        lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    for (int i = 0; i < EXIT_DIR_COUNT; ++i) {
        if (lowered == EXIT_NAMES[i]) {
            return i;
        }
    }
    return -1;
}

bool entity_matches(long vnum, const std::string& name, const std::string& query) {
    if (query.empty()) {
        return true;
    }

    auto contains_insensitive = [](const std::string& haystack, const std::string& needle) {
        if (needle.empty()) {
            return true;
        }
        auto lower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
        std::string hay;
        hay.reserve(haystack.size());
        for (char c : haystack) {
            hay.push_back(lower(static_cast<unsigned char>(c)));
        }
        std::string ned;
        ned.reserve(needle.size());
        for (char c : needle) {
            ned.push_back(lower(static_cast<unsigned char>(c)));
        }
        return hay.find(ned) != std::string::npos;
    };

    return contains_insensitive(std::to_string(vnum), query)
           || contains_insensitive(name, query);
}

long suggest_next_room_vnum(const World& world) {
    const long max_vnum = max_vnum_in_map(world.rooms);
    if (max_vnum > 0) {
        return max_vnum + 1;
    }
    return first_zone_bottom(world);
}

long suggest_next_mob_vnum(const World& world) {
    const long max_vnum = max_vnum_in_map(world.mobiles);
    return max_vnum > 0 ? max_vnum + 1 : 1;
}

long suggest_next_object_vnum(const World& world) {
    const long max_vnum = max_vnum_in_map(world.objects);
    return max_vnum > 0 ? max_vnum + 1 : 1;
}

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

bool create_room(World& world, long vnum, const RoomEdit& edit) {
    if (vnum <= 0 || world.rooms.count(vnum) > 0) {
        return false;
    }

    Room room;
    room.vnum = vnum;
    room.name = edit.name.empty() ? "Nuova stanza" : edit.name;
    room.description = edit.description.empty() ? "Una stanza vuota." : edit.description;
    room.sector_type = edit.sector_type >= 0 ? edit.sector_type : 1;
    room.room_flags = edit.room_flags >= 0 ? edit.room_flags : 0;
    room.zone_index = zone_index_for_vnum(world, vnum);
    world.rooms.emplace(vnum, std::move(room));
    return true;
}

bool create_mob(World& world, long vnum, const MobEdit& edit) {
    if (vnum <= 0 || world.mobiles.count(vnum) > 0) {
        return false;
    }
    world.mobiles.emplace(vnum, make_default_mobile(vnum, edit));
    return true;
}

bool create_object(World& world, long vnum, const ObjEdit& edit) {
    if (vnum <= 0 || world.objects.count(vnum) > 0) {
        return false;
    }
    world.objects.emplace(vnum, make_default_object(vnum, edit));
    return true;
}

const Exit* find_room_exit(const Room& room, int direction) {
    for (const auto& exit : room.exits) {
        if (exit.direction == direction) {
            return &exit;
        }
    }
    return nullptr;
}

bool set_room_exit(World& world, long room_vnum, const ExitEdit& edit) {
    Room* room = world.find_room(room_vnum);
    if (!room || edit.direction < 0 || edit.direction >= EXIT_DIR_COUNT) {
        return false;
    }

    Exit exit;
    exit.direction = edit.direction;
    exit.description = edit.description;
    exit.keyword = edit.keyword;
    exit.exit_info = edit.exit_info;
    exit.key = edit.key;
    exit.to_room = edit.to_room;
    exit.open_cmd = edit.open_cmd;

    for (auto& existing : room->exits) {
        if (existing.direction == edit.direction) {
            existing = exit;
            return true;
        }
    }
    room->exits.push_back(exit);
    return true;
}

bool remove_room_exit(World& world, long room_vnum, int direction) {
    Room* room = world.find_room(room_vnum);
    if (!room) {
        return false;
    }
    const auto it = std::remove_if(room->exits.begin(),
                                   room->exits.end(),
                                   [direction](const Exit& exit) {
                                       return exit.direction == direction;
                                   });
    if (it == room->exits.end()) {
        return false;
    }
    room->exits.erase(it, room->exits.end());
    return true;
}

} // namespace nebbie
