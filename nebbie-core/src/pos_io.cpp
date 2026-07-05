#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"
#include "nebbie/file_io.hpp"

#include <cstdio>

namespace nebbie {

namespace {

constexpr int kPoseClasses = 4;

void fwrite_action(FILE* fp, const std::string& value) {
    if (value.empty()) {
        std::fprintf(fp, "#\n");
    } else {
        std::fprintf(fp, "%s\n", value.c_str());
    }
}

} // namespace

void load_myst_pos(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    world.pose_entries.clear();

    FILE* fp = open_file_read(path, "pose file");
    if (progress) {
        progress("Loading " + path.string());
    }

    while (true) {
        PoseEntry entry;
        entry.level = static_cast<int>(fread_number(fp));
        if (entry.level < 0) {
            break;
        }
        skip_optional_blank_line(fp);

        for (int i = 0; i < kPoseClasses; ++i) {
            entry.poser_msg[i] = fread_action(fp);
            entry.room_msg[i] = fread_action(fp);
        }

        world.pose_entries.push_back(std::move(entry));
    }

    std::fclose(fp);
}

void save_myst_pos(const World& world, const std::filesystem::path& path, ProgressCallback progress) {
    FILE* fp = open_file_write(path, "pose file");
    if (progress) {
        progress("Writing " + path.string());
    }

    for (const auto& entry : world.pose_entries) {
        std::fprintf(fp, "%d\n", entry.level);
        for (int i = 0; i < kPoseClasses; ++i) {
            fwrite_action(fp, entry.poser_msg[i]);
            fwrite_action(fp, entry.room_msg[i]);
        }
    }

    std::fprintf(fp, "-1\n");
    std::fclose(fp);
}

} // namespace nebbie
