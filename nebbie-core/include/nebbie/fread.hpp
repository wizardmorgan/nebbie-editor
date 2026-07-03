#pragma once

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

namespace nebbie {

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& message) : std::runtime_error(message) {}
};

char fread_letter(FILE* fp);
void fread_to_eol(FILE* fp);
long fread_number(FILE* fp);
long fread_if_number(FILE* fp);
std::string fread_string(FILE* fp);
std::string fread_word(FILE* fp);
std::string fread_line(FILE* fp);
std::string fread_action(FILE* fp);
std::string fread_social_field(FILE* fp);
void skip_optional_blank_line(FILE* fp);
std::vector<long> parse_numbers(const std::string& line);

} // namespace nebbie
