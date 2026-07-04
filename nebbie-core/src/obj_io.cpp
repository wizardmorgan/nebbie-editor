#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"

#include <cstdio>
#include <cstring>
#include <sstream>

namespace nebbie {

namespace {

FILE* open_read(const std::filesystem::path& path) {
    FILE* fp = std::fopen(path.string().c_str(), "r");
    if (!fp) {
        throw ParseError("Unable to open object file: " + path.string());
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
        throw ParseError("Unable to write object file: " + path.string());
    }
    return fp;
}

std::string obj_context(const GameObject& obj) {
    std::string ctx = "obj #" + std::to_string(obj.vnum);
    if (!obj.short_descr.empty()) {
        ctx += " (" + obj.short_descr + ")";
    }
    return ctx;
}

std::string trim_line(std::string line) {
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

std::vector<long> parse_obj_numbers(const GameObject& obj, const std::string& line, const char* field) {
    try {
        return parse_numbers(line);
    } catch (const ParseError& ex) {
        throw ParseError(std::string(obj_context(obj)) + ": invalid " + field + " line \"" + line
                         + "\" (" + ex.what() + ")");
    }
}

long read_obj_number(FILE* fp, const GameObject& obj, const char* field) {
    try {
        return fread_number(fp);
    } catch (const ParseError& ex) {
        throw ParseError(std::string(obj_context(obj)) + ": invalid " + field + " (" + ex.what() + ")");
    }
}

char consume_section(FILE* fp) {
    int c = std::fgetc(fp);
    while (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
        c = std::fgetc(fp);
    }
    if (c == EOF) {
        return '\0';
    }
    return static_cast<char>(c);
}

char read_obj_marker(FILE* fp) {
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

std::string read_data_line(FILE* fp) {
    while (true) {
        const std::string line = trim_line(fread_line(fp));
        if (!line.empty() && line != "~") {
            return line;
        }
    }
}

char peek_section_letter(FILE* fp) {
    const long pos = std::ftell(fp);
    const char letter = consume_section(fp);
    std::fseek(fp, pos, SEEK_SET);
    return letter;
}

bool is_object_section_letter(char letter) {
    return letter == 'E' || letter == 'A' || letter == 'F' || letter == 'P';
}

void read_prescript_object_tail(FILE* fp, GameObject& obj) {
    while (true) {
        if (fread_peek_is_number(fp)) {
            return;
        }

        const char letter = peek_section_letter(fp);
        if (letter == 'E') {
            (void)consume_section(fp);
            ExtraDesc extra;
            extra.keyword = fread_string(fp);
            extra.description = fread_string(fp);
            obj.extra_descs.push_back(std::move(extra));
            continue;
        }
        if (is_object_section_letter(letter)) {
            return;
        }

        const std::string line = trim_line(peek_file_line(fp));
        if (line.empty() || line == "~") {
            (void)fread_line(fp);
            continue;
        }

        (void)fread_string(fp);
    }
}

void read_object_numeric_header(FILE* fp, GameObject& obj) {
    const auto type_line = parse_obj_numbers(obj, read_data_line(fp), "type");
    if (type_line.size() < 3) {
        throw ParseError(obj_context(obj) + ": expected at least 3 type fields");
    }
    obj.type_flag = static_cast<int>(type_line[0]);
    obj.extra_flags = type_line[1];
    obj.wear_flags = type_line[2];

    const auto value_line = parse_obj_numbers(obj, read_data_line(fp), "value");
    if (value_line.size() < 4) {
        throw ParseError(obj_context(obj) + ": expected 4 value fields");
    }
    for (int i = 0; i < 4; ++i) {
        obj.value[i] = static_cast<int>(value_line[i]);
    }

    const auto econ_line = parse_obj_numbers(obj, read_data_line(fp), "weight/cost");
    if (econ_line.size() < 3) {
        throw ParseError(obj_context(obj) + ": expected weight, cost, and cost_per_day");
    }
    obj.weight = static_cast<int>(econ_line[0]);
    obj.cost = static_cast<int>(econ_line[1]);
    obj.cost_per_day = static_cast<int>(econ_line[2]);
}

void read_object_entry(FILE* fp, GameObject& obj) {
    obj.name = fread_string(fp);
    obj.short_descr = fread_string(fp);
    obj.description = fread_string(fp);
    obj.action_description = fread_string(fp);

    obj.has_extra_flags2 = false;
    obj.extra_flags2 = 0;
    obj.forbidden_char.clear();
    obj.forbidden_room.clear();
    obj.extra_descs.clear();
    obj.affects.clear();

    read_prescript_object_tail(fp, obj);
    read_object_numeric_header(fp, obj);

    char section = consume_section(fp);
    while (true) {
        if (section == '\0' || section == '#' || section == '%') {
            if (section == '#' || section == '%') {
                std::ungetc(section, fp);
            }
            break;
        }

        if (section == 'E') {
            ExtraDesc extra;
            extra.keyword = fread_string(fp);
            extra.description = fread_string(fp);
            obj.extra_descs.push_back(std::move(extra));
            section = consume_section(fp);
            continue;
        }

        if (section == 'A') {
            const auto nums = parse_obj_numbers(obj, read_data_line(fp), "affect");
            if (nums.size() < 2) {
                throw ParseError(obj_context(obj) + ": expected affect location and modifier");
            }
            ObjAffect affect;
            affect.location = static_cast<int>(nums[0]);
            affect.modifier = static_cast<int>(nums[1]);
            obj.affects.push_back(affect);
            section = consume_section(fp);
            continue;
        }

        if (section == 'F') {
            obj.has_extra_flags2 = true;
            obj.extra_flags2 = read_obj_number(fp, obj, "extra_flags2");
            fread_to_eol(fp);
            section = consume_section(fp);
            continue;
        }

        if (section == 'P') {
            obj.forbidden_char = fread_string(fp);
            obj.forbidden_room = fread_string(fp);
            section = consume_section(fp);
            continue;
        }

        fread_to_eol(fp);
        section = consume_section(fp);
    }
}

void fwrite_string(FILE* fp, const std::string& value) {
    std::fprintf(fp, "%s~\n", value.c_str());
}

void write_object_entry(FILE* fp, const GameObject& obj) {
    std::fprintf(fp, "#%ld\n", obj.vnum);
    fwrite_string(fp, obj.name);
    fwrite_string(fp, obj.short_descr);
    fwrite_string(fp, obj.description);
    fwrite_string(fp, obj.action_description);

    std::fprintf(fp, "%d %ld %ld\n",
                 obj.type_flag, obj.extra_flags, obj.wear_flags);
    std::fprintf(fp, "%d %d %d %d\n",
                 obj.value[0], obj.value[1], obj.value[2], obj.value[3]);
    std::fprintf(fp, "%d %d %d\n",
                 obj.weight, obj.cost, obj.cost_per_day);

    for (const auto& extra : obj.extra_descs) {
        std::fprintf(fp, "E\n");
        fwrite_string(fp, extra.keyword);
        fwrite_string(fp, extra.description);
    }

    for (const auto& affect : obj.affects) {
        std::fprintf(fp, "A\n%d %d\n", affect.location, affect.modifier);
    }

    if (obj.has_extra_flags2) {
        std::fprintf(fp, "F\n%d\n", static_cast<int>(obj.extra_flags2));
    }

    if (!obj.forbidden_char.empty() || !obj.forbidden_room.empty()) {
        std::fprintf(fp, "P\n");
        fwrite_string(fp, obj.forbidden_char);
        fwrite_string(fp, obj.forbidden_room);
    }
}

} // namespace

void load_myst_obj(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    world.objects.clear();

    FILE* fp = open_read(path);
    if (progress) {
        progress("Loading " + path.string());
    }

    long last_loaded_vnum = -1;
    std::size_t loaded_count = 0;

    while (true) {
        const char marker = read_obj_marker(fp);
        if (marker == '%') {
            if (fread_letter(fp) != '%') {
                throw ParseError("Malformed object file terminator");
            }
            break;
        }
        if (marker != '#') {
            const std::string context = peek_file_line(fp);
            std::string message = "Expected # in myst.obj";
            if (last_loaded_vnum >= 0) {
                message += " after obj #" + std::to_string(last_loaded_vnum) + " ("
                           + std::to_string(loaded_count) + " objects loaded)";
            }
            message += " — found '" + std::string(1, marker) + "'";
            if (!context.empty()) {
                message += " near line: \"" + context + "\"";
            }
            throw ParseError(message);
        }

        if (!fread_peek_is_number(fp)) {
            fread_to_eol(fp);
            continue;
        }

        GameObject obj;
        try {
            obj.vnum = fread_number(fp);
        } catch (const ParseError& ex) {
            std::string message = "reading obj vnum";
            if (last_loaded_vnum >= 0) {
                message += " after obj #" + std::to_string(last_loaded_vnum) + " ("
                           + std::to_string(loaded_count) + " objects loaded)";
            }
            const std::string nearby = peek_file_line(fp);
            if (!nearby.empty()) {
                message += " near line: \"" + nearby + "\"";
            }
            message += ": " + std::string(ex.what());
            throw ParseError(message);
        }
        if (obj.vnum >= 99999) {
            break;
        }
        try {
            read_object_entry(fp, obj);
        } catch (const ParseError& ex) {
            const std::string detail = ex.what();
            if (detail.find("obj #") == std::string::npos) {
                throw ParseError(obj_context(obj) + ": " + detail);
            }
            throw;
        }
        const long saved_vnum = obj.vnum;
        world.objects.emplace(obj.vnum, std::move(obj));
        last_loaded_vnum = saved_vnum;
        ++loaded_count;
    }

    std::fclose(fp);
}

void save_myst_obj(const World& world, const std::filesystem::path& path, ProgressCallback progress) {
    FILE* fp = open_write(path);
    if (progress) {
        progress("Writing " + path.string());
    }

    for (const auto& [vnum, obj] : world.objects) {
        (void)vnum;
        write_object_entry(fp, obj);
    }

    std::fprintf(fp, "%%%%\n");
    std::fclose(fp);
}

} // namespace nebbie
