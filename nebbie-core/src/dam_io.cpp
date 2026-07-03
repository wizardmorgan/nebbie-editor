#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"

#include <cstdio>

namespace nebbie {

namespace {

FILE* open_read(const std::filesystem::path& path) {
    FILE* fp = std::fopen(path.string().c_str(), "r");
    if (!fp) {
        throw ParseError("Unable to open damage file: " + path.string());
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
        throw ParseError("Unable to write damage file: " + path.string());
    }
    return fp;
}

void fwrite_string(FILE* fp, const std::string& value) {
    std::fprintf(fp, "%s~\n", value.c_str());
}

void read_combat_message(FILE* fp, CombatMessage& msg) {
    msg.attack_type = static_cast<int>(fread_number(fp));
    msg.die_attacker = fread_string(fp);
    msg.die_victim = fread_string(fp);
    msg.die_room = fread_string(fp);
    msg.miss_attacker = fread_string(fp);
    msg.miss_victim = fread_string(fp);
    msg.miss_room = fread_string(fp);
    msg.hit_attacker = fread_string(fp);
    msg.hit_victim = fread_string(fp);
    msg.hit_room = fread_string(fp);
    msg.god_attacker = fread_string(fp);
    msg.god_victim = fread_string(fp);
    msg.god_room = fread_string(fp);
}

void write_combat_message(FILE* fp, const CombatMessage& msg) {
    std::fprintf(fp, "M\n %d\n", msg.attack_type);
    fwrite_string(fp, msg.die_attacker);
    fwrite_string(fp, msg.die_victim);
    fwrite_string(fp, msg.die_room);
    fwrite_string(fp, msg.miss_attacker);
    fwrite_string(fp, msg.miss_victim);
    fwrite_string(fp, msg.miss_room);
    fwrite_string(fp, msg.hit_attacker);
    fwrite_string(fp, msg.hit_victim);
    fwrite_string(fp, msg.hit_room);
    fwrite_string(fp, msg.god_attacker);
    fwrite_string(fp, msg.god_victim);
    fwrite_string(fp, msg.god_room);
}

} // namespace

void load_myst_dam(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    world.damage_messages.clear();

    FILE* fp = open_read(path);
    if (progress) {
        progress("Loading " + path.string());
    }

    std::string marker = fread_word(fp);
    while (marker == "M") {
        CombatMessage msg;
        read_combat_message(fp, msg);
        world.damage_messages.push_back(std::move(msg));
        marker = fread_word(fp);
    }

    std::fclose(fp);
}

void save_myst_dam(const World& world, const std::filesystem::path& path, ProgressCallback progress) {
    FILE* fp = open_write(path);
    if (progress) {
        progress("Writing " + path.string());
    }

    for (const auto& msg : world.damage_messages) {
        write_combat_message(fp, msg);
    }

    std::fprintf(fp, "$~\n");
    std::fclose(fp);
}

} // namespace nebbie
