


#include <QApplication>
#include <memory>
#include <qapplication.h>
#include <windows/mainwindow.h>
#include <utils/ort_inf.h>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    ort_inferer* ort = new ort_inferer("model.onnx");

    mainwindow main_mainwindow;
    main_mainwindow.show();
    return app.exec();
}