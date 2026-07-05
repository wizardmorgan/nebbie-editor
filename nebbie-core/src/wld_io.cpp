#include "nebbie/io.hpp"
#include "nebbie/overlay_io.hpp"

#include "nebbie/fread.hpp"

#include <cstdio>
#include <cstdlib>

namespace nebbie {

namespace {

constexpr long SECT_WATER_NOSWIM = 7;
constexpr long SECT_UNDERWATER = 8;
constexpr long TUNNEL = 64;
constexpr long TELE_COUNT = 1;

FILE* open_read(const std::filesystem::path& path) {
    FILE* fp = std::fopen(path.string().c_str(), "r");
    if (!fp) {
        throw ParseError("Unable to open world file: " + path.string());
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
        throw ParseError("Unable to write world file: " + path.string());
    }
    return fp;
}

std::string trim_line(std::string line) {
    while (!line.empty() && (line.back() == ' ' || line.back() == '\t')) {
        line.pop_back();
    }
    std::size_t start = 0;
    while (start < line.size() && (line[start] == ' ' || line[start] == '\t')) {
        ++start;
    }
    return line.substr(start);
}

std::string read_data_line(FILE* fp) {
    while (true) {
        const std::string line = trim_line(fread_line(fp));
        if (!line.empty() && line != "~") {
            return line;
        }
    }
}

void read_exit(FILE* fp, Room& room, int direction) {
    Exit exit;
    exit.direction = direction;
    exit.description = fread_string(fp);
    exit.keyword = fread_string(fp);
    exit.exit_info = fread_number(fp);
    exit.key = fread_number(fp);
    exit.to_room = fread_number(fp);

    int c = std::fgetc(fp);
    if (c != EOF && (c == '-' || std::isdigit(c))) {
        std::ungetc(c, fp);
        exit.open_cmd = fread_number(fp);
    } else if (c != EOF) {
        std::ungetc(c, fp);
        exit.open_cmd = -1;
    }

    room.exits.push_back(exit);
}

void read_room_zone_line(FILE* fp, Room& room) {
    const auto nums = parse_numbers(read_data_line(fp));
    if (nums.size() < 3) {
        throw ParseError("Room " + std::to_string(room.vnum) + ": expected zone, flags, and sector");
    }

  (void)nums[0];
    room.room_flags = nums[1];
    const long sector_field = nums[2];

    room.tele_time = 0;
    room.tele_targ = 0;
    room.tele_mask = 0;
    room.tele_cnt = 0;

    if (sector_field != -1) {
        room.sector_type = static_cast<int>(sector_field);
        return;
    }

    if (nums.size() >= 7) {
        room.tele_time = nums[3];
        room.tele_targ = nums[4];
        room.tele_mask = nums[5];
        room.sector_type = static_cast<int>(nums[6]);
        if (nums.size() >= 8) {
            room.tele_cnt = nums[6];
            room.sector_type = static_cast<int>(nums[7]);
        }
        return;
    }

    if (nums.size() == 4) {
        room.tele_time = nums[3];
        room.sector_type = 0;
        return;
    }

    throw ParseError("Room " + std::to_string(room.vnum) + ": invalid teleport zone line");
}

void read_room_body(FILE* fp, Room& room, World& world) {
    room.name = fread_string(fp);
    room.description = fread_string(fp);

    read_room_zone_line(fp, room);

    const long sector = room.sector_type;

    if (sector == SECT_WATER_NOSWIM || sector == SECT_UNDERWATER) {
        room.river_speed = fread_if_number(fp);
        room.river_dir = fread_if_number(fp);
    }

    if (room.room_flags & TUNNEL) {
        room.moblim = fread_if_number(fp);
        if (room.moblim < 1) {
            room.moblim = 1;
        }
    }

    if (const Zone* zone = world.zone_for_vnum(room.vnum)) {
        for (size_t i = 0; i < world.zones.size(); ++i) {
            if (&world.zones[i] == zone) {
                room.zone_index = static_cast<int>(i);
                break;
            }
        }
    }

    char token[161];
    while (std::fscanf(fp, " %160s", token) == 1) {
        switch (token[0]) {
        case 'D':
            read_exit(fp, room, std::atoi(token + 1));
            break;
        case 'E': {
            ExtraDesc extra;
            extra.keyword = fread_string(fp);
            extra.description = fread_string(fp);
            room.extra_descs.push_back(extra);
            break;
        }
        case 'L':
            room.bright_at_night = fread_string(fp);
            room.bright_at_day = fread_string(fp);
            break;
        case 'S':
            return;
        case 'C':
            fread_to_eol(fp);
            break;
        default:
            fread_to_eol(fp);
            break;
        }
    }

    throw ParseError("Room " + std::to_string(room.vnum) + " missing terminating S");
}

void write_room_body(FILE* fp, const Room& room, const World& world) {
    std::fprintf(fp, "%s~\n", room.name.c_str());
    std::fprintf(fp, "%s~\n", room.description.c_str());

    const int zone_num = room.zone_index >= 0 && room.zone_index < static_cast<int>(world.zones.size())
        ? world.zones[room.zone_index].num
        : 0;

    if (room.tele_time || room.tele_targ || room.tele_mask) {
        std::fprintf(fp, "%d %ld -1\n", zone_num, room.room_flags);
        std::fprintf(fp, "%ld %ld %ld", room.tele_time, room.tele_targ, room.tele_mask);
        if (room.tele_mask & TELE_COUNT) {
            std::fprintf(fp, " %ld", room.tele_cnt);
        }
        std::fprintf(fp, " %ld\n", room.sector_type);
    } else {
        std::fprintf(fp, "%d %ld %ld\n", zone_num, room.room_flags, room.sector_type);
    }

    if (room.sector_type == SECT_WATER_NOSWIM || room.sector_type == SECT_UNDERWATER) {
        if (room.river_speed || room.river_dir) {
            std::fprintf(fp, "%ld %ld\n", room.river_speed, room.river_dir);
        }
    }

    if (room.room_flags & TUNNEL) {
        std::fprintf(fp, "%ld\n", room.moblim > 0 ? room.moblim : 1);
    }

    for (const auto& exit : room.exits) {
        std::fprintf(fp, "D%d\n", exit.direction);
        std::fprintf(fp, "%s~\n", exit.description.c_str());
        std::fprintf(fp, "%s~\n", exit.keyword.c_str());
        std::fprintf(fp, "%ld %ld %ld %ld\n",
                     exit.exit_info,
                     exit.key,
                     exit.to_room,
                     exit.open_cmd);
    }

    for (const auto& extra : room.extra_descs) {
        std::fprintf(fp, "E\n");
        std::fprintf(fp, "%s~\n", extra.keyword.c_str());
        std::fprintf(fp, "%s~\n", extra.description.c_str());
    }

    if (!room.bright_at_night.empty() || !room.bright_at_day.empty()) {
        std::fprintf(fp, "L\n");
        std::fprintf(fp, "%s~\n", room.bright_at_night.c_str());
        std::fprintf(fp, "%s~\n", room.bright_at_day.c_str());
    }

    std::fprintf(fp, "S\n");
}

char read_wld_marker(FILE* fp) {
    while (true) {
        int c = std::fgetc(fp);
        if (c == EOF) {
            return '\0';
        }
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            continue;
        }
        if (c == '*') {
            fread_to_eol(fp);
            continue;
        }
        return static_cast<char>(c);
    }
}

bool peek_is_eof(FILE* fp) {
    const long pos = std::ftell(fp);
    int c = std::fgetc(fp);
    while (c != EOF && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
        c = std::fgetc(fp);
    }
    const bool eof = (c == EOF);
    std::fseek(fp, pos, SEEK_SET);
    return eof;
}

} // namespace

void load_myst_wld(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    FILE* fp = open_read(path);
    if (progress) {
        progress("Loading " + path.string());
    }

    while (true) {
        const char marker = read_wld_marker(fp);
        if (marker == '\0') {
            break;
        }
        if (marker != '#') {
            fread_to_eol(fp);
            continue;
        }

        if (!fread_peek_is_number(fp)) {
            fread_to_eol(fp);
            continue;
        }

        const long vnum = fread_number(fp);
        if (vnum == 0 && peek_is_eof(fp)) {
            break;
        }

        Room room;
        room.vnum = vnum;
        read_room_body(fp, room, world);
        world.rooms[vnum] = std::move(room);
    }

    std::fclose(fp);
}

void save_myst_wld(const World& world, const std::filesystem::path& path, ProgressCallback progress) {
    if (progress) {
        progress("Saving " + path.string());
    }

    FILE* fp = open_write(path);
    for (const auto& [vnum, room] : world.rooms) {
        (void)vnum;
        std::fprintf(fp, "#%ld\n", room.vnum);
        write_room_body(fp, room, world);
    }
    std::fprintf(fp, "#0\n");
    std::fclose(fp);
}

void save_room_overlay(const Room& room, const World& world, const std::filesystem::path& path) {
    FILE* fp = open_write(path);
    write_room_body(fp, room, world);
    std::fclose(fp);
}

void load_room_overlay(World& world, const long vnum, const std::filesystem::path& path) {
    FILE* fp = open_read(path);
    Room room;
    room.vnum = vnum;
    read_room_body(fp, room, world);
    std::fclose(fp);
    world.rooms[vnum] = std::move(room);
}

} // namespace nebbie
