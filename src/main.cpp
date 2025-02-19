


#include <QApplication>
#include <memory>
#include <opencv2/imgcodecs.hpp>
#include <qapplication.h>
#include <windows/mainwindow.h>
#include <utils/ort_inf.h>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    ort_inferer* ort = new ort_inferer("model.onnx");

    cv::Mat test = cv::imread("");

    std::cout << "infer result: " << ort->infer(test);

    mainwindow main_mainwindow;
    main_mainwindow.show();
    return app.exec();
}