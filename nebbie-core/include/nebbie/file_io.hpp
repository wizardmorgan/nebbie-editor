#pragma once

#include <cstdio>
#include <filesystem>
#include <string>

namespace nebbie {

std::string path_to_utf8(const std::filesystem::path& path);

FILE* open_file_read(const std::filesystem::path& path, const char* label);
FILE* open_file_write(const std::filesystem::path& path, const char* label);

} // namespace nebbie
