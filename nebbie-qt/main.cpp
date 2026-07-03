#include "main_window.hpp"

#include <QApplication>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("Nebbie Editor");
    app.setOrganizationName("Nebbie");

    MainWindow window;
    if (argc >= 2) {
        window.openLibPath(QString::fromUtf8(argv[1]));
    }
    window.show();

    return app.exec();
}
