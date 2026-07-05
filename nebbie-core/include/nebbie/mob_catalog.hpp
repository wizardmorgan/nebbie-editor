#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace nebbie {

struct MobFlagDef {
    long value = 0;
    const char* name = "";
    const char* label = "";
};

struct DiceValues {
    int number = 1;
    int size = 1;
    int plus = 0;
};

std::vector<MobFlagDef> mob_act_flags();
std::vector<MobFlagDef> mob_affected_flags();
std::vector<MobFlagDef> mob_immunity_flags();

std::vector<std::pair<int, std::string>> mob_position_choices();
std::vector<std::pair<int, std::string>> mob_sex_choices();
std::vector<std::pair<int, std::string>> mob_race_choices();
std::vector<std::pair<char, std::string>> mob_type_choices();

std::string mob_race_name(int race);
std::string mob_position_name(int position);
std::string mob_sex_name(int sex);
std::string mob_type_name(char mobtype);

bool mob_uses_hit_dice(char mobtype);
bool mob_type_uses_mult_att(char mobtype);
bool mob_type_uses_sounds(char mobtype);

DiceValues parse_dice(const std::string& text);
std::string format_dice(const DiceValues& dice);

long flags_value_from_selection(const std::vector<MobFlagDef>& defs, const std::vector<bool>& selected);
std::vector<bool> flags_selection_from_value(const std::vector<MobFlagDef>& defs, long value);

} // namespace nebbie
