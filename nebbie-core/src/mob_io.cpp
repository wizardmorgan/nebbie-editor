#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <vector>

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

bool mob_uses_hit_dice(char mobtype) {
    return mobtype == 'S';
}

std::string mob_context(const Mobile& mob) {
    std::string ctx = "mob #" + std::to_string(mob.vnum);
    if (!mob.short_descr.empty()) {
        ctx += " (" + mob.short_descr + ")";
    }
    return ctx;
}

std::string trim_combat_line(std::string line) {
    while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t')) {
        line.pop_back();
    }
    std::size_t start = 0;
    while (start < line.size() && (line[start] == ' ' || line[start] == '\t')) {
        ++start;
    }
    if (start > 0) {
        line.erase(0, start);
    }
    return line;
}

std::vector<std::string> split_combat_tokens(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<std::string> normalize_combat_tokens(const std::vector<std::string>& tokens) {
    std::vector<std::string> normalized;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == "=") {
            if (i + 1 < tokens.size()) {
                std::size_t hint = 0;
                if (tokens[i + 1][hint] == '+' || tokens[i + 1][hint] == '-') {
                    ++hint;
                }
                if (hint < tokens[i + 1].size()
                    && std::isdigit(static_cast<unsigned char>(tokens[i + 1][hint]))) {
                    ++i;
                }
            }
            continue;
        }

        const auto eq = tokens[i].find('=');
        if (eq != std::string::npos) {
            normalized.push_back(tokens[i].substr(0, eq));
        } else {
            normalized.push_back(tokens[i]);
        }
    }
    return normalized;
}

int parse_combat_int(const std::string& token) {
    if (token.empty()) {
        throw ParseError("empty combat number");
    }

    std::size_t index = 0;
    if (token[index] == '+' || token[index] == '-') {
        ++index;
    }
    if (index >= token.size() || !std::isdigit(static_cast<unsigned char>(token[index]))) {
        throw ParseError("invalid combat number token: \"" + token + "\"");
    }
    while (index < token.size() && std::isdigit(static_cast<unsigned char>(token[index]))) {
        ++index;
    }
    return std::stoi(token.substr(0, index));
}

bool peek_is_mob_boundary(FILE* fp);
void read_trailing_sound_strings(FILE* fp, Mobile& mob);
std::vector<long> parse_mob_numbers(const Mobile& mob, const std::string& line, const char* field);
long read_mob_number(FILE* fp, const Mobile& mob, const char* field);
std::string fread_rest_of_line(FILE* fp);

void parse_combat_line(const std::string& combat_line, Mobile& mob) {
    const auto tokens = normalize_combat_tokens(split_combat_tokens(combat_line));
    if (tokens.size() < 5) {
        throw ParseError(mob_context(mob) + ": invalid combat line (expected 5 fields): \"" + combat_line + "\"");
    }

    mob.level = parse_combat_int(tokens[0]);
    mob.hitroll = parse_combat_int(tokens[1]);
    mob.ac = parse_combat_int(tokens[2]);

    if (mob_uses_hit_dice(mob.mobtype)) {
        mob.hit_dice = tokens[3];
        mob.dam_dice = tokens[4];
        mob.hit_bonus = 0;
    } else {
        mob.hit_bonus = parse_combat_int(tokens[3]);
        mob.hit_dice.clear();
        mob.dam_dice = tokens[4];
    }
}

void read_new_mob_stats(FILE* fp, Mobile& mob) {
    const std::string combat_line = trim_combat_line(fread_line(fp));
    try {
        parse_combat_line(combat_line, mob);
    } catch (const ParseError&) {
        throw;
    } catch (const std::exception&) {
        throw ParseError(mob_context(mob) + ": invalid combat line: \"" + combat_line + "\"");
    }

    const auto gold_line = parse_mob_numbers(mob, fread_line(fp), "gold");
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

    const auto pos_line = parse_mob_numbers(mob, fread_line(fp), "position");
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

    read_trailing_sound_strings(fp, mob);
}

bool peek_is_mob_boundary(FILE* fp) {
    const long pos = std::ftell(fp);
    int c = std::fgetc(fp);
    while (c != EOF && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
        c = std::fgetc(fp);
    }
    const bool boundary = (c == '#' || c == '%');
    std::fseek(fp, pos, SEEK_SET);
    return boundary;
}

void read_trailing_sound_strings(FILE* fp, Mobile& mob) {
    mob.sounds.clear();
    mob.distant_sounds.clear();
    mob.extra_sound_strings.clear();

    std::vector<std::string> trailing;
    while (!peek_is_mob_boundary(fp)) {
        trailing.push_back(fread_string(fp));
    }

    if (!trailing.empty()) {
        mob.sounds = trailing[0];
    }
    if (trailing.size() > 1) {
        mob.distant_sounds = trailing[1];
    }
    if (trailing.size() > 2) {
        mob.extra_sound_strings.assign(trailing.begin() + 2, trailing.end());
    }
}

bool mob_has_trailing_sounds(const Mobile& mob) {
    return !mob.sounds.empty() || !mob.distant_sounds.empty() || !mob.extra_sound_strings.empty();
}

std::string fread_rest_of_line(FILE* fp) {
    std::string line;
    int c = std::fgetc(fp);
    while (c != EOF && c != '\n' && c != '\r') {
        line.push_back(static_cast<char>(c));
        c = std::fgetc(fp);
    }
    if (c == '\r') {
        c = std::fgetc(fp);
    }
    if (c != '\n' && c != EOF) {
        std::ungetc(c, fp);
    }
    return line;
}

std::vector<long> parse_mob_numbers(const Mobile& mob, const std::string& line, const char* field) {
    try {
        return parse_numbers(line);
    } catch (const ParseError& ex) {
        throw ParseError(std::string(mob_context(mob)) + ": invalid " + field + " line \"" + line
                         + "\" (" + ex.what() + ")");
    }
}

long read_mob_number(FILE* fp, const Mobile& mob, const char* field) {
    try {
        return fread_number(fp);
    } catch (const ParseError& ex) {
        throw ParseError(std::string(mob_context(mob)) + ": invalid " + field + " (" + ex.what() + ")");
    }
}

void read_mobile_entry(FILE* fp, Mobile& mob) {
    mob.name = fread_string(fp);
    mob.short_descr = fread_string(fp);
    mob.long_descr = fread_string(fp);
    mob.description = fread_string(fp);

    mob.act = read_mob_number(fp, mob, "act");
    mob.affected_by = read_mob_number(fp, mob, "affected_by");
    mob.alignment = read_mob_number(fp, mob, "alignment");

    mob.mobtype = fread_letter(fp);
    if (!is_new_mob_type(mob.mobtype)) {
        throw ParseError(mob_context(mob) + ": unsupported mob type '" + mob.mobtype
                         + "' (expected S, A, N, B, or L)");
    }

    const std::string type_tail = fread_rest_of_line(fp);
    if (mob.mobtype == 'A' || mob.mobtype == 'B' || mob.mobtype == 'L') {
        std::string tail = type_tail;
        while (!tail.empty() && (tail.front() == ' ' || tail.front() == '\t')) {
            tail.erase(tail.begin());
        }
        if (!tail.empty()) {
            const auto nums = parse_mob_numbers(mob, tail, "mult_att");
            mob.mult_att = nums.empty() ? 1 : static_cast<int>(nums[0]);
        } else {
            mob.mult_att = 1;
        }
    } else {
        mob.mult_att = 1;
    }

    read_new_mob_stats(fp, mob);
}

void fwrite_string(FILE* fp, const std::string& value) {
    std::fprintf(fp, "%s~\n", value.c_str());
}

void write_new_mob_stats(FILE* fp, const Mobile& mob) {
    if (mob_uses_hit_dice(mob.mobtype)) {
        std::fprintf(fp, "%d %d %d %s %s\n",
                     mob.level, mob.hitroll, mob.ac,
                     mob.hit_dice.c_str(), mob.dam_dice.c_str());
    } else {
        std::fprintf(fp, "%d %d %d %d %s\n",
                     mob.level, mob.hitroll, mob.ac,
                     mob.hit_bonus, mob.dam_dice.c_str());
    }

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

    if (mob.mobtype == 'L' || mob_has_trailing_sounds(mob)) {
        fwrite_string(fp, mob.sounds);
        fwrite_string(fp, mob.distant_sounds);
        for (const auto& extra : mob.extra_sound_strings) {
            fwrite_string(fp, extra);
        }
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

void skip_utf8_bom(FILE* fp) {
    const long pos = std::ftell(fp);
    unsigned char bom[3] = {};
    if (std::fread(bom, 1, 3, fp) == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
        return;
    }
    std::fseek(fp, pos, SEEK_SET);
}

char read_mob_marker(FILE* fp) {
    while (true) {
        const char marker = fread_letter(fp);
        if (marker == '*') {
            fread_to_eol(fp);
            continue;
        }
        return marker;
    }
}

std::string peek_file_line(FILE* fp) {
    const long pos = std::ftell(fp);
    const std::string line = fread_line(fp);
    std::fseek(fp, pos, SEEK_SET);
    return line;
}

} // namespace

void load_myst_mob(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    world.mobiles.clear();

    FILE* fp = open_read(path);
    if (progress) {
        progress("Loading " + path.string());
    }

    skip_utf8_bom(fp);

    long last_loaded_vnum = -1;
    std::size_t loaded_count = 0;

    while (true) {
        const char marker = read_mob_marker(fp);
        if (marker == '%') {
            if (fread_letter(fp) != '%') {
                throw ParseError("Malformed mob file terminator");
            }
            break;
        }
        if (marker != '#') {
            const std::string hint = (marker == '$')
                                         ? " (color/sound line? check type L mobs have two ~-terminated sound strings)"
                                         : "";
            const std::string context = peek_file_line(fp);
            std::string message = "Expected # in myst.mob";
            if (last_loaded_vnum >= 0) {
                message += " after mob #" + std::to_string(last_loaded_vnum)
                           + " (" + std::to_string(loaded_count) + " mobiles loaded)";
            }
            message += " — found '" + std::string(1, marker) + "'";
            if (!context.empty()) {
                message += " near line: \"" + context + "\"";
            }
            message += hint;
            throw ParseError(message);
        }

        Mobile mob;
        try {
            mob.vnum = fread_number(fp);
        } catch (const ParseError& ex) {
            std::string message = "reading mob vnum";
            if (last_loaded_vnum >= 0) {
                message += " after mob #" + std::to_string(last_loaded_vnum) + " ("
                           + std::to_string(loaded_count) + " mobiles loaded)";
            }
            const std::string nearby = peek_file_line(fp);
            if (!nearby.empty()) {
                message += " near line: \"" + nearby + "\"";
            }
            message += ": " + std::string(ex.what());
            throw ParseError(message);
        }
        if (mob.vnum >= 99999) {
            break;
        }
        try {
            read_mobile_entry(fp, mob);
        } catch (const ParseError& ex) {
            const std::string detail = ex.what();
            if (detail.find("mob #") == std::string::npos) {
                throw ParseError(mob_context(mob) + ": " + detail);
            }
            throw;
        }
        const long saved_vnum = mob.vnum;
        world.mobiles.emplace(mob.vnum, std::move(mob));
        last_loaded_vnum = saved_vnum;
        ++loaded_count;
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
