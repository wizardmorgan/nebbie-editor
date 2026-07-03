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

struct ObjAffect {
    int location = 0;
    int modifier = 0;
};

struct Mobile {
    long vnum = 0;
    std::string name;
    std::string short_descr;
    std::string long_descr;
    std::string description;
    long act = 0;
    long affected_by = 0;
    long alignment = 0;
    char mobtype = 'S';
    int mult_att = 1;
    int level = 0;
    int hitroll = 0;
    int ac = 0;
    int hit_bonus = 0;
    std::string hit_dice;
    std::string dam_dice;
    bool extended_gold = false;
    long gold = 0;
    long exp = 0;
    long race = 0;
    int position = 0;
    int default_pos = 0;
    int sex = 0;
    bool extended_sex = false;
    long immune = 0;
    long meta_immune = 0;
    long susceptible = 0;
    std::string sounds;
    std::string distant_sounds;
};

struct GameObject {
    long vnum = 0;
    std::string name;
    std::string short_descr;
    std::string description;
    std::string action_description;
    int type_flag = 0;
    long extra_flags = 0;
    long wear_flags = 0;
    int value[4] = {};
    int weight = 0;
    int cost = 0;
    int cost_per_day = 0;
    std::vector<ExtraDesc> extra_descs;
    std::vector<ObjAffect> affects;
    bool has_extra_flags2 = false;
    long extra_flags2 = 0;
    std::string forbidden_char;
    std::string forbidden_room;
};

constexpr int MAX_SHOP_PROD = 5;
constexpr int MAX_SHOP_TRADE = 5;

struct Shop {
    long vnum = 0;
    int producing[MAX_SHOP_PROD] = {};
    float profit_buy = 1.0f;
    float profit_sell = 1.0f;
    int trade_types[MAX_SHOP_TRADE] = {};
    std::string no_such_item1;
    std::string no_such_item2;
    std::string do_not_buy;
    std::string missing_cash1;
    std::string missing_cash2;
    std::string message_buy;
    std::string message_sell;
    int temper1 = 0;
    int temper2 = 0;
    int keeper = 0;
    int with_who = 0;
    int in_room = 0;
    int open1 = 0;
    int close1 = 0;
    int open2 = 0;
    int close2 = 0;
};

struct SpecialProc {
    char type = 0;
    long vnum = 0;
    std::string procedure;
    std::string params;
};

struct CombatMessage {
    int attack_type = 0;
    std::string die_attacker;
    std::string die_victim;
    std::string die_room;
    std::string miss_attacker;
    std::string miss_victim;
    std::string miss_room;
    std::string hit_attacker;
    std::string hit_victim;
    std::string hit_room;
    std::string god_attacker;
    std::string god_victim;
    std::string god_room;
};

struct SocialMessage {
    int act_nr = 0;
    int hide = 0;
    int min_victim_position = 0;
    std::string char_no_arg;
    std::string others_no_arg;
    std::string char_found;
    std::string others_found;
    std::string vict_found;
    std::string not_found;
    std::string char_auto;
    std::string others_auto;
};

struct PoseEntry {
    int level = 0;
    std::string poser_msg[4];
    std::string room_msg[4];
};

struct GuildEntry {
    std::string base_filename;
    int guard_mob = 0;
    int guard_room = 0;
    int guard_dir = 0;
    int banker_mob = 0;
    int bank_room = 0;
    int banker_xp_mob = 0;
    int bank_xp_room = 0;
    int member_book_obj = 0;
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
