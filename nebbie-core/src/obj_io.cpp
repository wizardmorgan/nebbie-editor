#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"

#include <cstdio>
#include <cstring>

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

void read_object_entry(FILE* fp, GameObject& obj) {
    obj.name = fread_string(fp);
    obj.short_descr = fread_string(fp);
    obj.description = fread_string(fp);
    obj.action_description = fread_string(fp);

    obj.type_flag = static_cast<int>(fread_number(fp));
    obj.extra_flags = fread_number(fp);
    obj.wear_flags = fread_number(fp);
    for (int i = 0; i < 4; ++i) {
        obj.value[i] = static_cast<int>(fread_number(fp));
    }
    obj.weight = static_cast<int>(fread_number(fp));
    obj.cost = static_cast<int>(fread_number(fp));
    obj.cost_per_day = static_cast<int>(fread_number(fp));

    char section = consume_section(fp);
    while (section == 'E') {
        ExtraDesc extra;
        extra.keyword = fread_string(fp);
        extra.description = fread_string(fp);
        obj.extra_descs.push_back(std::move(extra));
        section = consume_section(fp);
    }

    while (section == 'A') {
        ObjAffect affect;
        affect.location = static_cast<int>(fread_number(fp));
        affect.modifier = static_cast<int>(fread_number(fp));
        obj.affects.push_back(affect);
        fread_to_eol(fp);
        section = consume_section(fp);
    }

    obj.has_extra_flags2 = false;
    obj.extra_flags2 = 0;
    if (section == 'F') {
        obj.has_extra_flags2 = true;
        obj.extra_flags2 = fread_number(fp);
        fread_to_eol(fp);
        section = consume_section(fp);
    }

    if (section == 'P') {
        obj.forbidden_char = fread_string(fp);
        obj.forbidden_room = fread_string(fp);
        section = consume_section(fp);
    }

    if (section == '#') {
        std::ungetc('#', fp);
    } else if (section == '%') {
        std::ungetc('%', fp);
    } else if (section != '\0') {
        std::ungetc(section, fp);
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

    while (true) {
        const char marker = fread_letter(fp);
        if (marker == '%') {
            if (fread_letter(fp) != '%') {
                throw ParseError("Malformed object file terminator");
            }
            break;
        }
        if (marker != '#') {
            throw ParseError("Expected # in myst.obj");
        }

        GameObject obj;
        obj.vnum = fread_number(fp);
        if (obj.vnum >= 99999) {
            break;
        }
        read_object_entry(fp, obj);
        world.objects.emplace(obj.vnum, std::move(obj));
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
