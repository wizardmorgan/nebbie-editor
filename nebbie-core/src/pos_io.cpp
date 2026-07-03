#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"

#include <cstdio>

namespace nebbie {

namespace {

constexpr int kPoseClasses = 4;

FILE* open_read(const std::filesystem::path& path) {
    FILE* fp = std::fopen(path.string().c_str(), "r");
    if (!fp) {
        throw ParseError("Unable to open pose file: " + path.string());
    }
    return fp;
}

FILE* open_write(const std::filesystem::path& path) {
    std::error_code ec;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ec);
    }
    FILE* fp = std::fopen(path.string().c_str(), "w");
    if (!fp) {
        throw ParseError("Unable to write pose file: " + path.string());
    }
    return fp;
}

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

    FILE* fp = open_read(path);
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
    FILE* fp = open_write(path);
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
