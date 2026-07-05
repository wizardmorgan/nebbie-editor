#pragma once

#include "io.hpp"
#include "lib_context.hpp"
#include "world.hpp"

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace nebbie {

struct SessionConfig {
    int autosave_interval_sec = 10;
    int version_interval_sec = 60;
    std::size_t max_versions = 50;
};

struct VersionInfo {
    std::string id;
    std::string label;
    std::filesystem::file_time_type modified{};
};

std::filesystem::path session_root(const std::filesystem::path& lib_root);
std::filesystem::path workspace_dir(const std::filesystem::path& lib_root);
std::filesystem::path versions_dir(const std::filesystem::path& lib_root);

void save_snapshot(const World& world,
                   const LibContext& context,
                   const std::filesystem::path& destination,
                   const std::string& label = {});

void backup_lib_on_disk(const LibContext& context,
                        const std::filesystem::path& destination,
                        const std::string& label = "pre-save");

std::vector<VersionInfo> list_versions(const std::filesystem::path& lib_root);

std::string create_version(const World& world,
                           const LibContext& context,
                           const std::filesystem::path& lib_root,
                           const std::string& label);

void restore_snapshot(World& world,
                      LibContext& context,
                      const std::filesystem::path& snapshot_dir);

void prune_versions(const std::filesystem::path& lib_root, std::size_t max_versions);

struct AutosaveResult {
    bool workspace_updated = false;
    bool version_created = false;
    std::string version_id;
};

AutosaveResult run_autosave(const World& world,
                            const LibContext& context,
                            const std::filesystem::path& lib_root,
                            const SessionConfig& config,
                            std::chrono::system_clock::time_point last_version_time);

void save_lib_with_backup(const World& world,
                          const LibContext& context,
                          const std::filesystem::path& lib_root,
                          ProgressCallback progress = {});

} // namespace nebbie
