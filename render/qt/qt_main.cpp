#include "main_window.h"
#include "ui_main_window.h"

int qt_main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    MainWindow main_window;
    main_window.show();

    return a.exec(); // NOLINT(*-static-accessed-through-instance)
}