#pragma once

#include "mob_catalog.hpp"

#include <string>
#include <utility>
#include <vector>

namespace nebbie {

std::vector<std::pair<int, std::string>> room_sector_choices();
std::vector<MobFlagDef> room_flag_defs();
std::vector<MobFlagDef> exit_flag_defs();
std::vector<std::pair<int, std::string>> exit_direction_choices();

std::string room_sector_name(int sector_type);
std::string exit_direction_label(int direction);

bool room_sector_uses_river(int sector_type);
bool room_flags_use_moblim(long room_flags);

} // namespace nebbie
