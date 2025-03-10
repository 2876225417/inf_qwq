


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
#include <windows/mainwindow.h>
#include <utils/ort_inf.h>
#include <onnxruntime_cxx_api.h>

#include <utils/trt_inf.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    trt_inf::TRT_det_inferer* _trt_inferer = new trt_inf::TRT_det_inferer(); 
    

    // cv::Mat img = cv::imread("c1.jpg");
    //
    // det_inferer* det = new det_inferer();
    // rec_inferer* rec = new rec_inferer();
    //
    // std::vector<cv::Mat> croppeds = det->run_inf(img);
    //
    // for (auto& cropped: croppeds) rec->run_inf(cropped);
    // QApplication app(argc, argv);
    //
    // mainwindow main_mainwindow;
    // main_mainwindow.show();
    // return app.exec();
}


