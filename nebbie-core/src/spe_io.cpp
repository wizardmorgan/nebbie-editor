#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"

#include <cctype>
#include <cstdio>
#include <sstream>

namespace nebbie {

namespace {

FILE* open_read(const std::filesystem::path& path) {
    FILE* fp = std::fopen(path.string().c_str(), "r");
    if (!fp) {
        throw ParseError("Unable to open special proc file: " + path.string());
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
        throw ParseError("Unable to write special proc file: " + path.string());
    }
    return fp;
}

void trim(std::string& value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.erase(value.begin());
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
}

bool is_special_type(char c) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return c == 'm' || c == 'o' || c == 'r';
}

SpecialProc parse_special_line(const std::string& line) {
    if (line.empty() || line[0] == '*') {
        return {};
    }
    if (line == "$" || (line.size() == 1 && line[0] == '$')) {
        return {};
    }

    std::istringstream iss(line);
    std::string type_token;
    long vnum = 0;
    std::string procedure;

    iss >> type_token >> vnum >> procedure;
    if (type_token.empty() || procedure.empty() || !is_special_type(type_token[0])) {
        throw ParseError("Invalid special proc line: " + line);
    }

    std::string params;
    std::getline(iss, params);
    trim(params);

    SpecialProc entry;
    entry.type = static_cast<char>(std::tolower(static_cast<unsigned char>(type_token[0])));
    entry.vnum = vnum;
    entry.procedure = procedure;
    entry.params = params;
    return entry;
}

} // namespace

void load_myst_spe(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    world.special_procs.clear();

    FILE* fp = open_read(path);
    if (progress) {
        progress("Loading " + path.string());
    }

    while (true) {
        const std::string line = fread_line(fp);
        if (line.empty()) {
            if (std::feof(fp)) {
                break;
            }
            continue;
        }
        if (line == "$" || line == "$~") {
            break;
        }

        std::string trimmed = line;
        trim(trimmed);
        if (trimmed.empty() || trimmed[0] == '*') {
            continue;
        }

        const SpecialProc entry = parse_special_line(trimmed);
        if (entry.type == 0) {
            continue;
        }
        world.special_procs.push_back(entry);
    }

    std::fclose(fp);
}

void save_myst_spe(const World& world, const std::filesystem::path& path, ProgressCallback progress) {
    FILE* fp = open_write(path);
    if (progress) {
        progress("Writing " + path.string());
    }

    for (const auto& entry : world.special_procs) {
        const char type = static_cast<char>(std::toupper(static_cast<unsigned char>(entry.type)));
        if (entry.params.empty()) {
            std::fprintf(fp, "%c %ld %s\n", type, entry.vnum, entry.procedure.c_str());
        } else {
            std::fprintf(fp, "%c %ld %s %s\n",
                         type, entry.vnum, entry.procedure.c_str(), entry.params.c_str());
        }
    }

    std::fprintf(fp, "$\n");
    std::fclose(fp);
}

} // namespace nebbie
