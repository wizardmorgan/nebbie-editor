#pragma once

#include "nebbie/edit.hpp"

#include <QString>

namespace nebbie::qt {

QString application_log_path();
void append_application_log(const QString& message);
QString format_exit_alignment_report(const ExitAlignmentReport& report,
                                     const QString& context,
                                     const QString& library_path);

} // namespace nebbie::qt
