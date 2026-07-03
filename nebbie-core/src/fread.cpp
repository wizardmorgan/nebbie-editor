#include "nebbie/fread.hpp"

#include <cctype>
#include <cstdlib>
#include <sstream>

namespace nebbie {

namespace {

bool is_space(int c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

} // namespace

char fread_letter(FILE* fp) {
    int c;
    do {
        c = std::fgetc(fp);
        if (c == EOF) {
            throw ParseError("Unexpected end of file while reading letter");
        }
    } while (is_space(c));
    return static_cast<char>(c);
}

void fread_to_eol(FILE* fp) {
    int c;
    do {
        c = std::fgetc(fp);
    } while (c != '\n' && c != '\r' && c != EOF);

    do {
        c = std::fgetc(fp);
    } while (c == '\n' || c == '\r');
    if (c != EOF) {
        std::ungetc(c, fp);
    }
}

long fread_number(FILE* fp) {
    int c;
    do {
        c = std::fgetc(fp);
        if (c == EOF) {
            throw ParseError("Unexpected end of file while reading number");
        }
    } while (is_space(c));

    long number = 0;
    bool sign = false;

    if (c == '+') {
        c = std::fgetc(fp);
    } else if (c == '-') {
        sign = true;
        c = std::fgetc(fp);
    }

    if (!std::isdigit(c)) {
        throw ParseError("Bad number format");
    }

    while (std::isdigit(c)) {
        number = number * 10 + (c - '0');
        c = std::fgetc(fp);
    }

    if (sign) {
        number = -number;
    }

    if (c == '|') {
        number += fread_number(fp);
    } else if (c != ' ' && c != EOF) {
        std::ungetc(c, fp);
    }

    return number;
}

long fread_if_number(FILE* fp) {
    int c = std::fgetc(fp);
    if (c == EOF) {
        return 0;
    }
    if (!std::isdigit(c) && c != '-' && c != '+') {
        std::ungetc(c, fp);
        return 0;
    }
    std::ungetc(c, fp);
    return fread_number(fp);
}

std::string fread_word(FILE* fp) {
    std::string word;
    int c;

    do {
        c = std::fgetc(fp);
        if (c == EOF) {
            throw ParseError("Unexpected end of file while reading word");
        }
    } while (is_space(c));

    while (!is_space(c) && c != EOF) {
        word.push_back(static_cast<char>(c));
        c = std::fgetc(fp);
    }

    if (c != EOF) {
        std::ungetc(c, fp);
    }

    return word;
}

std::string fread_line(FILE* fp) {
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

void skip_optional_blank_line(FILE* fp) {
    const long pos = std::ftell(fp);
    const std::string line = fread_line(fp);
    if (!line.empty()) {
        std::fseek(fp, pos, SEEK_SET);
    }
}

namespace {

bool is_social_header_line(const std::string& line) {
    if (line.empty() || line[0] == '#') {
        return false;
    }
    if (line.find('$') != std::string::npos) {
        return false;
    }

    std::istringstream iss(line);
    int values[3] = {};
    for (int& value : values) {
        if (!(iss >> value)) {
            return false;
        }
    }

    std::string extra;
    return !(iss >> extra);
}

} // namespace

std::string fread_social_field(FILE* fp) {
    const long pos = std::ftell(fp);
    std::string line = fread_line(fp);
    if (line.empty() || line[0] == '#') {
        return {};
    }
    if (is_social_header_line(line)) {
        std::fseek(fp, pos, SEEK_SET);
        return {};
    }
    return line;
}

std::string fread_action(FILE* fp) {
    std::string line = fread_line(fp);
    if (line.empty() || line[0] == '#') {
        return {};
    }
    return line;
}

namespace {

long parse_number_token(const std::string& token, std::size_t& index) {
    long value = 0;
    bool sign = false;

    if (index < token.size() && token[index] == '+') {
        ++index;
    } else if (index < token.size() && token[index] == '-') {
        sign = true;
        ++index;
    }

    if (index >= token.size() || !std::isdigit(static_cast<unsigned char>(token[index]))) {
        throw ParseError("Bad number format");
    }

    while (index < token.size() && std::isdigit(static_cast<unsigned char>(token[index]))) {
        value = value * 10 + (token[index] - '0');
        ++index;
    }

    if (index < token.size() && token[index] == '|') {
        ++index;
        value += parse_number_token(token, index);
    }

    return sign ? -value : value;
}

} // namespace

std::vector<long> parse_numbers(const std::string& line) {
    std::vector<long> values;
    std::size_t index = 0;

    while (index < line.size()) {
        while (index < line.size() && is_space(static_cast<unsigned char>(line[index]))) {
            ++index;
        }
        if (index >= line.size()) {
            break;
        }
        values.push_back(parse_number_token(line, index));
    }

    return values;
}

std::string fread_string(FILE* fp) {
    std::string result;
    int c = std::fgetc(fp);

    if (c == EOF) {
        throw ParseError("Unexpected end of file while reading string");
    }

    while (c == '\r' || c == '\n') {
        c = std::fgetc(fp);
        if (c == EOF) {
            throw ParseError("Unexpected end of file while reading string");
        }
    }

    if (c == '~') {
        return result;
    }

    while (true) {
        if (c == '~') {
            break;
        }
        if (c == '\r') {
            c = std::fgetc(fp);
            continue;
        }
        if (c == EOF) {
            throw ParseError("Unterminated string");
        }
        result.push_back(static_cast<char>(c));
        c = std::fgetc(fp);
    }

    return result;
}

} // namespace nebbie
