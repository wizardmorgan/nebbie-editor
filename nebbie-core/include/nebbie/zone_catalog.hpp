#pragma once

#include <string>
#include <utility>
#include <vector>

namespace nebbie {

struct ResetArgLabels {
    std::string if_flag;
    std::string arg1;
    std::string arg2;
    std::string arg3;
    std::string arg4;
};

std::vector<std::pair<int, std::string>> zone_reset_mode_choices();
std::vector<std::pair<char, std::string>> zone_reset_command_choices();
std::vector<std::pair<int, std::string>> zone_equip_position_choices();
std::vector<std::pair<int, std::string>> zone_door_state_choices();

std::string zone_reset_mode_label(int reset_mode);
std::string zone_reset_command_legend(char command);
ResetArgLabels zone_reset_arg_labels(char command);

} // namespace nebbie
