#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"

#include <cstdio>
#include <cstring>

namespace nebbie {

namespace {

FILE* open_read(const std::filesystem::path& path) {
    FILE* fp = std::fopen(path.string().c_str(), "r");
    if (!fp) {
        throw ParseError("Unable to open mob file: " + path.string());
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
        throw ParseError("Unable to write mob file: " + path.string());
    }
    return fp;
}

bool is_new_mob_type(char letter) {
    return letter == 'S' || letter == 'A' || letter == 'N' || letter == 'B' || letter == 'L';
}

void read_new_mob_stats(FILE* fp, Mobile& mob) {
    const auto combat_line = fread_line(fp);
    char hit_buf[128] = {};
    char dam_buf[128] = {};
    const int parsed = std::sscanf(combat_line.c_str(), "%d %d %d %127s %127s",
                                   &mob.level, &mob.hitroll, &mob.ac, hit_buf, dam_buf);
    if (parsed < 5) {
        throw ParseError("Invalid mob combat line");
    }
    mob.hit_dice = hit_buf;
    mob.dam_dice = dam_buf;

    const auto gold_line = parse_numbers(fread_line(fp));
    if (gold_line.empty()) {
        throw ParseError("Missing mob gold line");
    }
    if (gold_line[0] == -1) {
        mob.extended_gold = true;
        mob.gold = gold_line.size() > 1 ? gold_line[1] : 0;
        mob.exp = gold_line.size() > 2 ? gold_line[2] : 0;
        mob.race = gold_line.size() > 3 ? gold_line[3] : 0;
    } else {
        mob.extended_gold = false;
        mob.gold = gold_line[0];
        mob.exp = gold_line.size() > 1 ? gold_line[1] : 0;
        mob.race = 0;
    }

    const auto pos_line = parse_numbers(fread_line(fp));
    if (pos_line.size() < 3) {
        throw ParseError("Invalid mob position line");
    }

    mob.position = static_cast<int>(pos_line[0]);
    mob.default_pos = static_cast<int>(pos_line[1]);

    const long sex_field = pos_line[2];
    if (sex_field < 3) {
        mob.sex = static_cast<int>(sex_field);
        mob.extended_sex = false;
    } else if (sex_field < 6) {
        mob.sex = static_cast<int>(sex_field - 3);
        mob.extended_sex = true;
        mob.immune = pos_line.size() > 3 ? pos_line[3] : 0;
        mob.meta_immune = pos_line.size() > 4 ? pos_line[4] : 0;
        mob.susceptible = pos_line.size() > 5 ? pos_line[5] : 0;
    } else {
        mob.sex = 0;
        mob.extended_sex = false;
    }

    if (mob.mobtype == 'L') {
        mob.sounds = fread_string(fp);
        mob.distant_sounds = fread_string(fp);
    }
}

void read_mobile_entry(FILE* fp, Mobile& mob) {
    mob.name = fread_string(fp);
    mob.short_descr = fread_string(fp);
    mob.long_descr = fread_string(fp);
    mob.description = fread_string(fp);

    mob.act = fread_number(fp);
    mob.affected_by = fread_number(fp);
    mob.alignment = fread_number(fp);

    mob.mobtype = fread_letter(fp);
    if (!is_new_mob_type(mob.mobtype)) {
        throw ParseError(std::string("Unsupported mob type: ") + mob.mobtype);
    }

    if (mob.mobtype == 'A' || mob.mobtype == 'B' || mob.mobtype == 'L') {
        mob.mult_att = static_cast<int>(fread_number(fp));
    } else {
        mob.mult_att = 1;
    }

    fread_to_eol(fp);
    read_new_mob_stats(fp, mob);
}

void fwrite_string(FILE* fp, const std::string& value) {
    std::fprintf(fp, "%s~\n", value.c_str());
}

void write_new_mob_stats(FILE* fp, const Mobile& mob) {
    std::fprintf(fp, "%d %d %d %s %s\n",
                 mob.level, mob.hitroll, mob.ac,
                 mob.hit_dice.c_str(), mob.dam_dice.c_str());

    if (mob.extended_gold) {
        if (mob.race != 0 || mob.exp != 0) {
            std::fprintf(fp, "-1 %ld %ld %ld\n", mob.gold, mob.exp, mob.race);
        } else {
            std::fprintf(fp, "-1 %ld\n", mob.gold);
        }
    } else {
        std::fprintf(fp, "%ld %ld\n", mob.gold, mob.exp);
    }

    if (mob.extended_sex) {
        std::fprintf(fp, "%d %d %d %ld %ld %ld\n",
                     mob.position, mob.default_pos, mob.sex + 3,
                     mob.immune, mob.meta_immune, mob.susceptible);
    } else {
        std::fprintf(fp, "%d %d %d\n", mob.position, mob.default_pos, mob.sex);
    }

    if (mob.mobtype == 'L') {
        fwrite_string(fp, mob.sounds);
        fwrite_string(fp, mob.distant_sounds);
    }
}

void write_mobile_entry(FILE* fp, const Mobile& mob) {
    std::fprintf(fp, "#%ld\n", mob.vnum);
    fwrite_string(fp, mob.name);
    fwrite_string(fp, mob.short_descr);
    fwrite_string(fp, mob.long_descr);
    fwrite_string(fp, mob.description);

    if (mob.mobtype == 'A' || mob.mobtype == 'B' || mob.mobtype == 'L') {
        std::fprintf(fp, "%ld %ld %ld %c %d\n",
                     mob.act, mob.affected_by, mob.alignment,
                     mob.mobtype, mob.mult_att);
    } else {
        std::fprintf(fp, "%ld %ld %ld %c\n",
                     mob.act, mob.affected_by, mob.alignment, mob.mobtype);
    }

    write_new_mob_stats(fp, mob);
}

} // namespace

void load_myst_mob(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    world.mobiles.clear();

    FILE* fp = open_read(path);
    if (progress) {
        progress("Loading " + path.string());
    }

    while (true) {
        const char marker = fread_letter(fp);
        if (marker == '%') {
            if (fread_letter(fp) != '%') {
                throw ParseError("Malformed mob file terminator");
            }
            break;
        }
        if (marker != '#') {
            throw ParseError("Expected # in myst.mob");
        }

        Mobile mob;
        mob.vnum = fread_number(fp);
        if (mob.vnum >= 99999) {
            break;
        }
        read_mobile_entry(fp, mob);
        world.mobiles.emplace(mob.vnum, std::move(mob));
    }

    std::fclose(fp);
}

void save_myst_mob(const World& world, const std::filesystem::path& path, ProgressCallback progress) {
    FILE* fp = open_write(path);
    if (progress) {
        progress("Writing " + path.string());
    }

    for (const auto& [vnum, mob] : world.mobiles) {
        (void)vnum;
        write_mobile_entry(fp, mob);
    }

    std::fprintf(fp, "%%%%\n");
    std::fclose(fp);
}

} // namespace nebbie
