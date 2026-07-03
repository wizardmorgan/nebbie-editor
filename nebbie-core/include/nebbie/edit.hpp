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

struct ExitEdit {
    int direction = 0;
    std::string description;
    std::string keyword;
    long exit_info = 0;
    long key = -1;
    long to_room = 0;
    long open_cmd = -1;
};

constexpr int EXIT_DIR_COUNT = 6;

const char* exit_direction_name(int direction);
int exit_direction_from_name(const std::string& name);

bool entity_matches(long vnum, const std::string& name, const std::string& query);

long suggest_next_room_vnum(const World& world);
long suggest_next_mob_vnum(const World& world);
long suggest_next_object_vnum(const World& world);

void apply_room_edit(Room& room, const RoomEdit& edit);
void apply_mob_edit(Mobile& mob, const MobEdit& edit);
void apply_obj_edit(GameObject& obj, const ObjEdit& edit);

bool edit_room(World& world, long vnum, const RoomEdit& edit);
bool edit_mob(World& world, long vnum, const MobEdit& edit);
bool edit_object(World& world, long vnum, const ObjEdit& edit);

bool create_room(World& world, long vnum, const RoomEdit& edit = {});
bool create_mob(World& world, long vnum, const MobEdit& edit = {});
bool create_object(World& world, long vnum, const ObjEdit& edit = {});

bool set_room_exit(World& world, long room_vnum, const ExitEdit& edit);
bool remove_room_exit(World& world, long room_vnum, int direction);
const Exit* find_room_exit(const Room& room, int direction);

Zone* find_zone(World& world, int zone_num);
const Zone* find_zone(const World& world, int zone_num);

bool is_editable_reset_command(char command);
std::string reset_command_summary(const ResetCommand& cmd);
ResetCommand default_zone_reset(char command, const Zone& zone, const World& world);

bool add_zone_reset(World& world, int zone_num, const ResetCommand& cmd);
bool update_zone_reset(World& world, int zone_num, std::size_t index, const ResetCommand& cmd);
bool remove_zone_reset(World& world, int zone_num, std::size_t index);
bool move_zone_reset(World& world, int zone_num, std::size_t from_index, std::size_t to_index);

RoomEdit room_edit_from_flags(const std::map<std::string, std::string>& flags);
MobEdit mob_edit_from_flags(const std::map<std::string, std::string>& flags);
ObjEdit obj_edit_from_flags(const std::map<std::string, std::string>& flags);

} // namespace nebbie
