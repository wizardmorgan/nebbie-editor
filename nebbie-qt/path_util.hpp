#pragma once

#include <QString>
#include <filesystem>

namespace nebbie::qt {

inline std::filesystem::path path_from_qstring(const QString& path) {
#if defined(_WIN32)
    return std::filesystem::path(path.toStdWString());
#else
    return std::filesystem::path(path.toUtf8().constData());
#endif
}

inline QString qstring_from_path(const std::filesystem::path& path) {
#if defined(_WIN32)
    return QString::fromStdWString(path.native());
#else
    return QString::fromUtf8(path.u8string().c_str());
#endif
}

} // namespace nebbie::qt
