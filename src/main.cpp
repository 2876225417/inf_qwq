


#include <QApplication>
#include <memory>
#include <opencv2/imgcodecs.hpp>
#include <qapplication.h>
#include <windows/mainwindow.h>
#include <utils/ort_inf.h>

int main(int argc, char* argv[]) {

    // ort_inferer* ort = new ort_inferer();
    // cv::Mat img = cv::imread("25.jpg");
    // std::string res = ort->exec_inf(img);
    // std::cout << "Inf res: " << res;
    //
    QApplication app(argc, argv);

    mainwindow main_mainwindow;
    main_mainwindow.show();
    return app.exec();
}
