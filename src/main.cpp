


#include "utils/ort_inf.hpp"
#include <QApplication>
#include <algorithm>
#include <memory>
#include <onnxruntime_c_api.h>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <qapplication.h>
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


int main(int argc, char* argv[]) {
    

    // test model conversion
    //trt_inf::TRT_det_inferer* _trt_inferer = new trt_inf::TRT_det_inferer(); 
    

    // cv::Mat img = cv::imread("c1.jpg");
    //
    // det_inferer* det = new det_inferer();
    // rec_inferer* rec = new rec_inferer();
    //
    // std::vector<cv::Mat> croppeds = det->run_inf(img);
    //
    // for (auto& cropped: croppeds) rec->run_inf(cropped);
    QApplication app(argc, argv);
    
    //if (!create_connection()) {
        
    //}

    mainwindow main_mainwindow;
    main_mainwindow.show();
    return app.exec();
}


