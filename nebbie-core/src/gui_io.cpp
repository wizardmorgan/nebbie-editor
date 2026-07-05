#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"
#include "nebbie/file_io.hpp"

#include <cstdio>
#include <cstring>

namespace nebbie {

namespace {

bool parse_guild_line(const std::string& line, GuildEntry& entry) {
    if (line.empty()) {
        return false;
    }

    char base[256] = {};
    int guard_mob = 0;
    int guard_room = 0;
    int guard_dir = 0;
    int banker_mob = 0;
    int bank_room = 0;
    int banker_xp_mob = 0;
    int bank_xp_room = 0;
    int member_book = 0;

    const int count = std::sscanf(line.c_str(),
                                  "%255s %d %d %d %d %d %d %d %d",
                                  base,
                                  &guard_mob,
                                  &guard_room,
                                  &guard_dir,
                                  &banker_mob,
                                  &bank_room,
                                  &banker_xp_mob,
                                  &bank_xp_room,
                                  &member_book);
    if (count != 9) {
        return false;
    }

    entry.base_filename = base;
    entry.guard_mob = guard_mob;
    entry.guard_room = guard_room;
    entry.guard_dir = guard_dir;
    entry.banker_mob = banker_mob;
    entry.bank_room = bank_room;
    entry.banker_xp_mob = banker_xp_mob;
    entry.bank_xp_room = bank_xp_room;
    entry.member_book_obj = member_book;
    return true;
}

} // namespace

void load_myst_gui(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    world.guilds.clear();

    FILE* fp = open_file_read(path, "guild file");
    if (progress) {
        progress("Loading " + path.string());
    }

    while (true) {
        std::string line = fread_line(fp);
        if (line.empty() && std::feof(fp)) {
            break;
        }

        GuildEntry entry;
        if (parse_guild_line(line, entry)) {
            world.guilds.push_back(std::move(entry));
        }
    }

    std::fclose(fp);
}

void save_myst_gui(const World& world, const std::filesystem::path& path, ProgressCallback progress) {
    FILE* fp = open_file_write(path, "guild file");
    if (progress) {
        progress("Writing " + path.string());
    }

    for (const auto& guild : world.guilds) {
        std::fprintf(fp,
                     "%s %d %d %d %d %d %d %d %d\n",
                     guild.base_filename.c_str(),
                     guild.guard_mob,
                     guild.guard_room,
                     guild.guard_dir,
                     guild.banker_mob,
                     guild.bank_room,
                     guild.banker_xp_mob,
                     guild.bank_xp_room,
                     guild.member_book_obj);
    }

    std::fclose(fp);
}

} // namespace nebbie
