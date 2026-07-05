#pragma once

#include "mob_catalog.hpp"

#include <string>
#include <utility>
#include <vector>

namespace nebbie {

std::vector<std::pair<int, std::string>> obj_type_choices();
std::vector<MobFlagDef> obj_wear_flags();
std::vector<MobFlagDef> obj_extra_flags();
std::vector<MobFlagDef> obj_extra_flags2();
std::vector<std::pair<int, std::string>> obj_apply_type_choices();

std::string obj_type_name(int type_flag);
std::string obj_apply_type_name(int location);

} // namespace nebbie
