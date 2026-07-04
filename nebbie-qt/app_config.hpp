#pragma once

#include <QString>

namespace nebbie::qt {

QString default_config_path();
QString read_lib_path();
bool write_lib_path(const QString& path);
bool lib_path_exists(const QString& path);

} // namespace nebbie::qt
