


#include <QApplication>
#include <qapplication.h>
#include <windows/mainwindow.h>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    mainwindow main_mainwindow;
    main_mainwindow.show();
    return app.exec();
}