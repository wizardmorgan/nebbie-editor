#pragma once

#include <filesystem>
#include <map>

namespace nebbiedit {

int run_shell(const std::filesystem::path& lib_root);

bool run_room_set(const std::filesystem::path& lib_root,
                  long vnum,
                  const std::map<std::string, std::string>& flags,
                  bool force);

bool run_mob_set(const std::filesystem::path& lib_root,
                 long vnum,
                 const std::map<std::string, std::string>& flags,
                 bool force);

bool run_obj_set(const std::filesystem::path& lib_root,
                 long vnum,
                 const std::map<std::string, std::string>& flags,
                 bool force);

} // namespace nebbiedit
