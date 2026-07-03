#pragma once

#include "world.hpp"

#include <filesystem>
#include <functional>
#include <string>

namespace nebbie {

using ProgressCallback = std::function<void(const std::string&)>;

void load_myst_zon(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_wld(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_mob(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_obj(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_lib(World& world, const std::filesystem::path& lib_root, ProgressCallback progress = {});

void save_myst_zon(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_wld(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_mob(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_obj(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});

} // namespace nebbie
