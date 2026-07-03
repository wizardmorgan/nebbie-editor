#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace nebbie {

struct ExtraDesc {
    std::string keyword;
    std::string description;
};

struct Exit {
    int direction = 0;
    std::string description;
    std::string keyword;
    long exit_info = 0;
    long key = 0;
    long to_room = 0;
    long open_cmd = -1;
};

struct Room {
    long vnum = 0;
    int zone_index = 0;
    std::string name;
    std::string description;
    long room_flags = 0;
    long sector_type = 0;
    long tele_time = 0;
    long tele_targ = 0;
    long tele_mask = 0;
    long tele_cnt = 0;
    long river_speed = 0;
    long river_dir = 0;
    long moblim = 0;
    std::string bright_at_night;
    std::string bright_at_day;
    std::vector<Exit> exits;
    std::vector<ExtraDesc> extra_descs;
};

struct ResetCommand {
    char command = 0;
    int if_flag = 0;
    int arg1 = -1;
    int arg2 = 0;
    int arg3 = -1;
    int arg4 = 0;
    std::string raw_line;
};

struct Zone {
    int num = 0;
    std::string name;
    int bottom = 0;
    int top = 0;
    int lifespan = 15;
    int reset_mode = 2;
    std::vector<ResetCommand> commands;
};

struct LibPaths {
    std::string root;
    std::string zon;
    std::string wld;
    std::string mob;
    std::string obj;
    std::string shp;
    std::string spe;
    std::string dam;
    std::string act;
    std::string pos;
    std::string gui;
};

} // namespace nebbie
