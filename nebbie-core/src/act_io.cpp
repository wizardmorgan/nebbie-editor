#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"
#include "nebbie/file_io.hpp"

#include <cstdio>

namespace nebbie {

namespace {

void fwrite_action(FILE* fp, const std::string& value) {
    if (value.empty()) {
        std::fprintf(fp, "#\n");
    } else {
        std::fprintf(fp, "%s\n", value.c_str());
    }
}

void skip_act_separators(FILE* fp) {
    while (true) {
        const long pos = std::ftell(fp);
        const std::string word = fread_word(fp);
        if (word == "#") {
            continue;
        }
        std::fseek(fp, pos, SEEK_SET);
        return;
    }
}

void read_social_message(FILE* fp, SocialMessage& msg) {
    msg.hide = static_cast<int>(fread_number(fp));
    msg.min_victim_position = static_cast<int>(fread_number(fp));
    skip_optional_blank_line(fp);

    msg.char_no_arg = fread_action(fp);
    msg.others_no_arg = fread_action(fp);
    msg.char_found = fread_action(fp);

    if (msg.char_found.empty()) {
        skip_act_separators(fp);
        return;
    }

    msg.others_found = fread_social_field(fp);
    msg.vict_found = fread_social_field(fp);
    msg.not_found = fread_social_field(fp);
    msg.char_auto = fread_social_field(fp);
    msg.others_auto = fread_social_field(fp);
    skip_act_separators(fp);
}

void write_social_message(FILE* fp, const SocialMessage& msg) {
    std::fprintf(fp, "%d %d %d\n", msg.act_nr, msg.hide, msg.min_victim_position);
    fwrite_action(fp, msg.char_no_arg);
    fwrite_action(fp, msg.others_no_arg);
    fwrite_action(fp, msg.char_found);

    if (msg.char_found.empty()) {
        return;
    }

    fwrite_action(fp, msg.others_found);
    fwrite_action(fp, msg.vict_found);
    fwrite_action(fp, msg.not_found);
    fwrite_action(fp, msg.char_auto);
    fwrite_action(fp, msg.others_auto);
}

} // namespace

void load_myst_act(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    world.social_messages.clear();

    FILE* fp = open_file_read(path, "social file");
    if (progress) {
        progress("Loading " + path.string());
    }

    while (true) {
        skip_act_separators(fp);
        SocialMessage msg;
        msg.act_nr = static_cast<int>(fread_number(fp));
        if (msg.act_nr < 0) {
            break;
        }

        read_social_message(fp, msg);

        world.social_messages.push_back(std::move(msg));
    }

    std::fclose(fp);
}

void save_myst_act(const World& world, const std::filesystem::path& path, ProgressCallback progress) {
    FILE* fp = open_file_write(path, "social file");
    if (progress) {
        progress("Writing " + path.string());
    }

    for (const auto& msg : world.social_messages) {
        write_social_message(fp, msg);
        std::fprintf(fp, "\n");
    }

    std::fprintf(fp, "-1\n");
    std::fclose(fp);
}

} // namespace nebbie
