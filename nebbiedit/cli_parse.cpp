#include "cli_parse.hpp"

#include <sstream>

namespace nebbiedit {

std::vector<std::string> split_command_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (c == '"') {
            in_quotes = !in_quotes;
            continue;
        }
        if (!in_quotes && (c == ' ' || c == '\t')) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }
        current.push_back(c);
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}

std::map<std::string, std::string> parse_flags(const std::vector<std::string>& args,
                                               std::size_t start) {
    std::map<std::string, std::string> flags;
    for (std::size_t i = start; i < args.size(); ++i) {
        const std::string& token = args[i];
        if (token.size() < 3 || token.rfind("--", 0) != 0) {
            continue;
        }
        const std::string key = token.substr(2);
        if (i + 1 < args.size() && args[i + 1].rfind("--", 0) != 0) {
            flags[key] = args[i + 1];
            ++i;
        } else {
            flags[key] = "1";
        }
    }
    return flags;
}

} // namespace nebbiedit
