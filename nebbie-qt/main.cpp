#include "main_window.hpp"
#include "app_config.hpp"
#include "path_util.hpp"

#include <QCoreApplication>
#include <QApplication>
#include <QIcon>
#include <QTimer>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("Nebbie Editor");
    app.setOrganizationName("Nebbie");
    app.setWindowIcon(QIcon(":/app-icon.png"));

    MainWindow window;
    window.show();

    const QStringList args = QCoreApplication::arguments();
    if (args.size() >= 2) {
        window.openLibPath(args.at(1));
    } else {
        QTimer::singleShot(0, &window, &MainWindow::openStartupLib);
    }

    return app.exec();
}
