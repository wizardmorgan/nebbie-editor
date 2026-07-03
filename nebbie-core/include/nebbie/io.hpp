#pragma once

#include "world.hpp"
#include "lib_context.hpp"

#include <filesystem>
#include <functional>
#include <string>

namespace nebbie {

using ProgressCallback = std::function<void(const std::string&)>;

void load_myst_zon(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_wld(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_mob(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_obj(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_shp(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_spe(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_dam(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_act(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_pos(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_myst_gui(World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void load_lib(World& world, const std::filesystem::path& lib_root, ProgressCallback progress = {});
void load_lib(World& world, const std::filesystem::path& lib_root, LibContext& context,
              ProgressCallback progress = {});
void save_lib(const World& world, const LibContext& context, ProgressCallback progress = {});

void save_myst_zon(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_wld(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_mob(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_obj(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_shp(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_spe(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_dam(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_act(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_pos(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});
void save_myst_gui(const World& world, const std::filesystem::path& path, ProgressCallback progress = {});

} // namespace nebbie
