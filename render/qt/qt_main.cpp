#include "ui_main_window.h"

int qt_main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    QMainWindow window;

    Ui_AudiPi mainWindow{};
    mainWindow.setupUi(&window);

    window.show();

    return a.exec();
}