#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"

#include <cstdio>
#include <cstring>

namespace nebbie {

namespace {

FILE* open_read(const std::filesystem::path& path) {
    FILE* fp = std::fopen(path.string().c_str(), "r");
    if (!fp) {
        throw ParseError("Unable to open zone file: " + path.string());
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
        throw ParseError("Unable to write zone file: " + path.string());
    }
    return fp;
}

bool is_reset_command(char c) {
    return std::strchr("HFMCOGEPD*;SR", c) != nullptr;
}

} // namespace

void load_myst_zon(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    world.zones.clear();

    FILE* fp = open_read(path);
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

        while (true) {
            int c = std::fgetc(fp);
            while (c != EOF && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
                c = std::fgetc(fp);
            }
            if (c == EOF) {
                throw ParseError("Unexpected EOF in zone reset table");
            }

            const char command = static_cast<char>(c);
            if (command == 'S') {
                fread_to_eol(fp);
                break;
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

        world.zones.push_back(zone);
    }

    std::fclose(fp);
}

void save_myst_zon(const World& world, const std::filesystem::path& path, ProgressCallback progress) {
    if (progress) {
        progress("Saving " + path.string());
    }

    FILE* fp = open_write(path);
    for (const auto& zone : world.zones) {
        std::fprintf(fp, "#%d\n", zone.num);
        std::fprintf(fp, "%s~\n", zone.name.c_str());
        std::fprintf(fp, "%d %d %d\n", zone.top, zone.lifespan, zone.reset_mode);

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
    std::fprintf(fp, "#$\n");
    std::fclose(fp);
}

} // namespace nebbie
