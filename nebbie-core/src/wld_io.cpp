#include "nebbie/io.hpp"

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

void read_room_body(FILE* fp, Room& room, World& world) {
    room.name = fread_string(fp);
    room.description = fread_string(fp);

    (void)fread_number(fp);

    room.room_flags = fread_number(fp);
    long sector = fread_number(fp);
    room.sector_type = sector;

    if (sector == -1) {
        room.tele_time = fread_number(fp);
        room.tele_targ = fread_number(fp);
        room.tele_mask = fread_number(fp);
        if (room.tele_mask & TELE_COUNT) {
            room.tele_cnt = fread_number(fp);
        }
        room.sector_type = fread_number(fp);
        sector = room.sector_type;
    }

    if (sector == SECT_WATER_NOSWIM || sector == SECT_UNDERWATER) {
        room.river_speed = fread_if_number(fp);
        room.river_dir = fread_if_number(fp);
    }

    if (room.room_flags & TUNNEL) {
        room.moblim = fread_number(fp);
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

} // namespace

void load_myst_wld(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    FILE* fp = open_read(path);
    if (progress) {
        progress("Loading " + path.string());
    }

    while (true) {
        if (fread_letter(fp) != '#') {
            throw ParseError("Expected # in myst.wld");
        }

        const long vnum = fread_number(fp);
        if (vnum == 0) {
            break;
        }

        Room room;
        room.vnum = vnum;
        read_room_body(fp, room, world);
        world.rooms[vnum] = room;
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
    std::fprintf(fp, "#0\n");
    std::fclose(fp);
}

} // namespace nebbie
