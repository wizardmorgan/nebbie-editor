#include "nebbie/file_io.hpp"

#include "nebbie/fread.hpp"

#include <system_error>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace nebbie {

std::string path_to_utf8(const std::filesystem::path& path) {
#if defined(_WIN32)
    const std::wstring native = path.native();
    if (native.empty()) {
        return {};
    }
    const int size = WideCharToMultiByte(CP_UTF8, 0, native.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }
    std::string utf8(static_cast<std::size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, native.c_str(), -1, utf8.data(), size, nullptr, nullptr);
    return utf8;
#else
    return path.u8string();
#endif
}

FILE* open_file_read(const std::filesystem::path& path, const char* label) {
#if defined(_WIN32)
    FILE* fp = _wfopen(path.c_str(), L"rb");
#else
    FILE* fp = std::fopen(path.u8string().c_str(), "rb");
#endif
    if (!fp) {
        throw ParseError(std::string("Unable to open ") + label + ": " + path_to_utf8(path));
    }
    return fp;
}

FILE* open_file_write(const std::filesystem::path& path, const char* label) {
    std::error_code ec;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ec);
    }
#if defined(_WIN32)
    FILE* fp = _wfopen(path.c_str(), L"wb");
#else
    FILE* fp = std::fopen(path.u8string().c_str(), "wb");
#endif
    if (!fp) {
        throw ParseError(std::string("Unable to write ") + label + ": " + path_to_utf8(path));
    }
    return fp;
}

} // namespace nebbie
