#include "app_config.hpp"

#include "nebbie/io.hpp"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

#include <filesystem>

namespace nebbie::qt {

namespace {

QString config_directory() {
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (base.isEmpty()) {
        base = QDir::homePath() + QStringLiteral("/.config/Nebbie");
    }
    QDir().mkpath(base);
    return base;
}

} // namespace

QString default_config_path() {
    return config_directory() + QStringLiteral("/nebbieedit.conf");
}

QString read_lib_path() {
    QFile file(default_config_path());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        if (line.startsWith(QStringLiteral("lib_path="))) {
            return line.mid(QStringLiteral("lib_path=").size()).trimmed();
        }
    }
    return {};
}

bool write_lib_path(const QString& path) {
    QFile file(default_config_path());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    out << "# Nebbie Editor configuration\n";
    out << "lib_path=" << path << '\n';
    return true;
}

bool lib_path_exists(const QString& path) {
    if (path.isEmpty()) {
        return false;
    }

    std::error_code ec;
    const std::filesystem::path candidate(path.toStdString());
    if (!std::filesystem::exists(candidate, ec)) {
        return false;
    }

    const std::filesystem::path resolved = nebbie::resolve_lib_directory(candidate);
    return std::filesystem::exists(resolved / "myst.zon", ec)
           || std::filesystem::exists(resolved / "myst.wld", ec)
           || std::filesystem::exists(resolved / "myst.mob", ec)
           || std::filesystem::exists(resolved / "myst.obj", ec);
}

} // namespace nebbie::qt
