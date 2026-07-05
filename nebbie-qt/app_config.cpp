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

QString value_after_equals(const QString& line) {
    const int eq = line.indexOf('=');
    if (eq < 0) {
        return {};
    }
    return line.mid(eq + 1).trimmed();
}

} // namespace

QString default_config_path() {
    return config_directory() + QStringLiteral("/nebbieedit.conf");
}

AppConfig read_config() {
    AppConfig config;
    QFile file(default_config_path());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return config;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        if (line.startsWith(QStringLiteral("lib_path="))) {
            config.lib_path = value_after_equals(line);
        } else if (line.startsWith(QStringLiteral("index_url="))) {
            config.index_url = value_after_equals(line);
        } else if (line.startsWith(QStringLiteral("coordinator_url="))) {
            config.coordinator_url = value_after_equals(line);
        } else if (line.startsWith(QStringLiteral("coordinator_token="))) {
            config.coordinator_token = value_after_equals(line);
        } else if (line.startsWith(QStringLiteral("builder_name="))) {
            config.builder_name = value_after_equals(line);
        }
    }
    return config;
}

bool write_config(const AppConfig& config) {
    QFile file(default_config_path());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    out << "# Nebbie Editor configuration\n";
    out << "lib_path=" << config.lib_path << '\n';
    if (!config.index_url.isEmpty()) {
        out << "index_url=" << config.index_url << '\n';
    }
    if (!config.coordinator_url.isEmpty()) {
        out << "coordinator_url=" << config.coordinator_url << '\n';
    }
    if (!config.coordinator_token.isEmpty()) {
        out << "coordinator_token=" << config.coordinator_token << '\n';
    }
    if (!config.builder_name.isEmpty()) {
        out << "builder_name=" << config.builder_name << '\n';
    }
    return true;
}

QString read_lib_path() {
    return read_config().lib_path;
}

bool write_lib_path(const QString& path) {
    AppConfig config = read_config();
    config.lib_path = path;
    return write_config(config);
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
