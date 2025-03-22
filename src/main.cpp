


#include "HCNetSDK.h"
#include "utils/ort_inf.hpp"
#include <QApplication>
#include <algorithm>
#include <chrono>
#include <memory>
#include <memory_resource>
#include <onnxruntime_c_api.h>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <qapplication.h>
#include <qcoreapplication.h>
#include <qsqldatabase.h>
#include <windows/mainwindow.h>
#include <utils/ort_inf.h>

#include <utils/trt_inf.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>

#include <utils/chars_ort_inferer.h>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv); 
    mainwindow main_mainwindow;
    main_mainwindow.show();
    return app.exec();
}


