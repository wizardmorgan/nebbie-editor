#include "main_window.hpp"
#include "app_config.hpp"

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

    if (argc >= 2) {
        window.openLibPath(QString::fromUtf8(argv[1]));
    } else {
        QTimer::singleShot(0, &window, &MainWindow::openStartupLib);
    }

    return app.exec();
}
