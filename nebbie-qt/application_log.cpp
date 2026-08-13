#include "application_log.hpp"

#include "path_util.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

namespace nebbie::qt {

QString application_log_path() {
    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(base);
    return base + QStringLiteral("/nebbieedit.log");
}

void append_application_log(const QString& message) {
    QFile file(application_log_path());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    for (const QString& line : message.split('\n')) {
        if (line.trimmed().isEmpty()) {
            stream << '\n';
            continue;
        }
        stream << '[' << timestamp << "] " << line << '\n';
    }
}

QString format_exit_alignment_report(const ExitAlignmentReport& report,
                                     const QString& context,
                                     const QString& library_path) {
    QStringList lines;
    lines << context;
    if (!library_path.isEmpty()) {
        lines << QStringLiteral("Libreria: %1").arg(library_path);
    }
    lines << QStringLiteral("Uscite controllate: %1").arg(static_cast<qlonglong>(report.exits_checked));
    lines << QStringLiteral("Già allineate: %1").arg(static_cast<qlonglong>(report.exits_already_ok));
    lines << QStringLiteral("Allineate: %1").arg(static_cast<qlonglong>(report.exits_aligned));
    lines << QStringLiteral("Destinazione mancante: %1")
                 .arg(static_cast<qlonglong>(report.exits_missing_destination));

    if (!report.changes.empty()) {
        lines << QStringLiteral("");
        lines << QStringLiteral("Dettaglio modifiche:");
        for (const auto& change : report.changes) {
            lines << QStringLiteral("  Stanza #%1 uscita %2 -> #%3: \"%4\" -> \"%5\"")
                         .arg(change.from_vnum)
                         .arg(QString::fromUtf8(nebbie::exit_direction_name(change.direction)))
                         .arg(change.to_vnum)
                         .arg(QString::fromStdString(change.old_description))
                         .arg(QString::fromStdString(change.new_description));
        }
    } else {
        lines << QStringLiteral("");
        lines << QStringLiteral("Nessuna modifica necessaria.");
    }

    return lines.join('\n');
}

} // namespace nebbie::qt
