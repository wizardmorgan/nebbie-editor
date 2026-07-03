#pragma once

#include <map>
#include <string>
#include <vector>

namespace nebbiedit {

std::vector<std::string> split_command_line(const std::string& line);
std::map<std::string, std::string> parse_flags(const std::vector<std::string>& args, std::size_t start);

} // namespace nebbiedit
