


#include <QApplication>
#include <memory>
#include <opencv2/imgcodecs.hpp>
#include <qapplication.h>
#include <windows/mainwindow.h>
#include <utils/ort_inf.h>

int main(int argc, char* argv[]) {

    QApplication app(argc, argv);

    mainwindow main_mainwindow;
    main_mainwindow.show();
    return app.exec();
}
