#include "nebbie/io.hpp"
#include "nebbie/edit.hpp"
#include "nebbie/overlay_io.hpp"

#include "nebbie/fread.hpp"
#include "nebbie/file_io.hpp"

#include <cstdio>
#include <cstring>

namespace nebbie {

namespace {

bool is_reset_command(char c) {
    return std::strchr("HFMCOGEPD*;SR", c) != nullptr;
}

void read_zone_reset_commands(FILE* fp, Zone& zone) {
    zone.commands.clear();
    while (true) {
        int c = std::fgetc(fp);
        while (c != EOF && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
            c = std::fgetc(fp);
        }
        if (c == EOF) {
            throw ParseError("Unexpected EOF in zone reset overlay");
        }

        const char command = static_cast<char>(c);
        if (command == 'S') {
            fread_to_eol(fp);
            return;
        }

        if (!is_reset_command(command)) {
            fread_to_eol(fp);
            continue;
        }

        if (command == '*' || command == ';') {
            char line[512];
            if (!std::fgets(line, sizeof(line), fp)) {
                break;
            }
            if (command == '*') {
                ResetCommand comment;
                comment.command = '*';
                comment.raw_line = line;
                zone.commands.push_back(comment);
            }
            continue;
        }

        if (command == 'R') {
            fread_to_eol(fp);
            continue;
        }

        char line[512];
        if (!std::fgets(line, sizeof(line), fp)) {
            throw ParseError("Unexpected EOF reading zone reset line");
        }

        ResetCommand reset;
        reset.command = command;
        reset.raw_line = line;
        int tmp = 0;
        std::sscanf(line, " %d %d %d %d %d",
                    &tmp,
                    &reset.arg1,
                    &reset.arg2,
                    &reset.arg3,
                    &reset.arg4);
        reset.if_flag = tmp;
        zone.commands.push_back(reset);
    }
}

void write_zone_reset_commands(FILE* fp, const Zone& zone) {
    for (const auto& cmd : zone.commands) {
        if (cmd.command == '*') {
            std::fprintf(fp, "*%s", cmd.raw_line.c_str());
            if (cmd.raw_line.empty() || cmd.raw_line.back() != '\n') {
                std::fprintf(fp, "\n");
            }
            continue;
        }
        std::fprintf(fp, "%c %d %d %d %d %d\n",
                     cmd.command,
                     cmd.if_flag,
                     cmd.arg1,
                     cmd.arg2,
                     cmd.arg3,
                     cmd.arg4);
    }
    std::fprintf(fp, "S\n");
}

} // namespace

void load_myst_zon(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    world.zones.clear();

    FILE* fp = open_file_read(path, "zone file");
    if (progress) {
        progress("Loading " + path.string());
    }

    while (true) {
        if (fread_letter(fp) != '#') {
            throw ParseError("Expected # in myst.zon");
        }

        const int maybe_end = std::fgetc(fp);
        if (maybe_end == '$') {
            break;
        }
        if (maybe_end != EOF) {
            std::ungetc(maybe_end, fp);
        }

        const long znumber = fread_number(fp);
        std::string name = fread_string(fp);
        if (name == "$" || (!name.empty() && name[0] == '$')) {
            break;
        }

        Zone zone;
        zone.num = static_cast<int>(znumber);
        zone.name = name;
        zone.top = static_cast<int>(fread_number(fp));
        zone.lifespan = static_cast<int>(fread_number(fp));
        zone.reset_mode = static_cast<int>(fread_number(fp));
        zone.bottom = world.zones.empty() ? 0 : world.zones.back().top + 1;

        read_zone_reset_commands(fp, zone);

        world.zones.push_back(zone);
    }

    std::fclose(fp);
}

void save_myst_zon(const World& world, const std::filesystem::path& path, ProgressCallback progress) {
    if (progress) {
        progress("Saving " + path.string());
    }

    FILE* fp = open_file_write(path, "zone file");
    for (const auto& zone : world.zones) {
        std::fprintf(fp, "#%d\n", zone.num);
        std::fprintf(fp, "%s~\n", zone.name.c_str());
        std::fprintf(fp, "%d %d %d\n", zone.top, zone.lifespan, zone.reset_mode);
        write_zone_reset_commands(fp, zone);
    }
    std::fprintf(fp, "#$\n");
    std::fclose(fp);
}

void save_zone_reset_overlay(const Zone& zone, const std::filesystem::path& path) {
    FILE* fp = open_file_write(path, "zone file");
    write_zone_reset_commands(fp, zone);
    std::fclose(fp);
}

void load_zone_reset_overlay(World& world, const int zone_num, const std::filesystem::path& path) {
    Zone* zone = find_zone(world, zone_num);
    if (!zone) {
        throw ParseError("Zone #" + std::to_string(zone_num) + " not loaded");
    }
    FILE* fp = open_file_read(path, "zone file");
    read_zone_reset_commands(fp, *zone);
    std::fclose(fp);
}

} // namespace nebbie
