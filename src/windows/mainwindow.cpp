#include "components/camera_wrapper.h"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "utils/ort_inf.h"
#include <qimage.h>
#include <qpushbutton.h>
#include <windows/mainwindow.h>

#include <chrono>

mainwindow::mainwindow(QWidget* parent)
    : QMainWindow{parent}
    // mainwindow_layout
    , m_mainwindow_layout{new QWidget()}
    , m_mainwindow_layout_wrapper{new QHBoxLayout()}
    , m_camera_rel_layout{new QVBoxLayout()}
    // camera_layout
    , m_mainwindow_camera_layout{new QGroupBox("Camera")}
    , m_mainwindow_camera_layout_wrapper{new QHBoxLayout()}
    // --camera_cropped_layout
    , m_camera_cropped_layout{new QGroupBox("Cropped")}
    , m_camera_cropped_layout_wrapper{new QVBoxLayout()}
    , m_cropped_img_wrapper{new QHBoxLayout()}
    {
    // camera and actions
    m_camera = new camera_wrapper();
    m_actions_wrapper = new actions_wrapper();
    // croppeds
    m_4_croppeds_img = new cropped_wrapper<4>(this); 
    m_cropped_img_wrapper->addWidget(m_4_croppeds_img);
    // tool bar and status bar
    m_tool_bar = new tool_bar();
    addToolBar(Qt::TopToolBarArea, m_tool_bar);
    m_status_bar = new status_bar();
    setStatusBar(m_status_bar);

    m_inferer = new ort_inferer();
    m_inferer->set_intra_threads(1); 
    // test for ort inf
    connect(test_inf, &QPushButton::clicked, this, [this]() { 
        cv::Mat mat = qimage2mat(tmp);
        cv::imshow("ts", mat);
        cv::imwrite("123.jpg", mat);
        cv::Mat local = cv::imread("123.jpg");
        std::string res = m_inferer->exec_inf(mat);
        qDebug() << "res: " << res;
        // if (mat.empty()) { qDebug() << "Mat is empty!"; return; }
        // std::string res = inferer->exec_inf(tmp);
        // qDebug() << "";
    });
    connect ( m_camera
            , &camera_wrapper::img_cropped
            , this
            , [this](QVector<cropped_image>& cropped_images) {
                qDebug() << "count of images: " << cropped_images.size();
                tmp = cropped_images[0].image;
                cv::Mat mat = qimage2mat(tmp);
                cv::imshow("tmp", mat);
                for (auto& cropped: cropped_images){
                    m_4_croppeds_img->set_image(cropped.number - 1, cropped.image);  qDebug() << "Images set"; }
             });

    connect ( m_camera, &camera_wrapper::img_cropped4inf
            , this, [this](QVector<cropped_image>& cropped_images) {
                auto start = std::chrono::high_resolution_clock::now();
                int idx = 0;
                for (auto cropped: cropped_images) {
                    idx += 1;
                    qDebug() << "推理结果: " << m_inferer->exec_inf(qimage2mat(cropped.image));  
               }
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                qDebug() << "推理总耗时： " << duration;
            });

    m_camera_cropped_layout->setLayout(m_cropped_img_wrapper);

    m_mainwindow_camera_layout_wrapper->addWidget(m_camera);
    m_mainwindow_camera_layout_wrapper->setAlignment(m_camera, Qt::AlignLeft | Qt::AlignTop);
    m_mainwindow_camera_layout_wrapper->setContentsMargins(0, 0, 0, 0);
    m_mainwindow_camera_layout->setLayout(m_mainwindow_camera_layout_wrapper);
    m_mainwindow_camera_layout->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_camera_rel_layout->addWidget(m_mainwindow_camera_layout);     // camera
    m_camera_rel_layout->addWidget(m_actions_wrapper);              // actions
    m_mainwindow_layout_wrapper->addLayout(m_camera_rel_layout);    // (camera + actions) wrapper
    m_mainwindow_layout_wrapper->addWidget(m_camera_cropped_layout);// camera croppeds
    
    m_mainwindow_layout->setLayout(m_mainwindow_layout_wrapper);    // mainwindow  
    resize(1400, 700);  // windows size w: 1400 h: 700
    setCentralWidget(m_mainwindow_layout);
}

mainwindow::~mainwindow() {
       
}

cv::Mat mainwindow::qimage2mat(QImage& qimage) {
    QImage swapped = qimage.convertToFormat(QImage::Format_RGB888)
                    .rgbSwapped();
    cv::Mat mat = cv::Mat(swapped.height(), swapped.width(), 
                        CV_8UC3, 
                        const_cast<uchar*>(swapped.constBits()), 
                        static_cast<size_t>(swapped.bytesPerLine()))
                .clone();  
    return mat;
}

