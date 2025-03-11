#include "components/actions_wrapper.h"
#include "components/camera_wrapper.h"
#include "components/cropped_wrapper.h"
#include "components/grouping_rtsp_stream.h"
#include "components/tool_bar.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "utils/ort_inf.h"
#include "utils/ort_inf.hpp"
#include "windows/rtsp_config_window.h"
#include <qgridlayout.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qsharedpointer.h>
#include <qsplitter.h>
#include <qstackedwidget.h>
#include <windows/mainwindow.h>

#include <chrono>

#include <components/grouping_sidebar.h>
#include <QStackedWidget>

#include <QSplitter>
#include <windows/expanded_camera_window.h>


mainwindow::mainwindow(QWidget* parent)
    : QMainWindow{parent}
    // mainwindow_layout
    , m_mainwindow_layout{new QWidget()}
    , m_mainwindow_layout_wrapper{new QHBoxLayout()}
    , m_camera_rel_layout{new QVBoxLayout()}
    , m_camera_rel_layout_grid{new QGridLayout()}
    // camera_layout
    , m_mainwindow_camera_layout{new QGroupBox("Camera")}
    , m_mainwindow_camera_layout_wrapper{new QHBoxLayout()}
    // --camera_cropped_layout
    , m_camera_cropped_layout{new QGroupBox("采集区")}
    , m_camera_cropped_layout_wrapper{new QVBoxLayout()}
    , m_cropped_img_wrapper{new QHBoxLayout()}
    {
  //
 
   //  // update cropped img 
   //  connect ( m_camera
   //          , &camera_wrapper::img_cropped
   //          , this
   //          , [this](QVector<cropped_image>& cropped_images) {
   //              qDebug() << "count of images: " << cropped_images.size();
   //              tmp = cropped_images[0].image;
   //              cv::Mat mat = qimage2mat(tmp);
   //              
   //              for (auto& cropped: cropped_images){
   //                  m_1_cropped_img->set_image(0, cropped.image);
   //                  m_4_croppeds_img->set_image(cropped.number - 1, cropped.image); }
   //           });
   // 

   //
   //
   //  // keyword
   //  connect ( m_actions_wrapper, &actions_wrapper::keywords_changed
   //          , this, [this](const QString& keywords) {
   //              qDebug() << "Keywords: " << keywords;
   //              m_1_cropped_img->set_keywords(keywords); 
   //           });
   //
   //
   //  connect (m_actions_wrapper, &actions_wrapper::start_inf, this, [this]() {
   //              auto start = std::chrono::high_resolution_clock::now();
   //              
   //              int idx = 0;
   //              std::vector<cv::Mat> croppeds = m_chars_det_inferer->run_inf(for2);
   //              QString result_set;
   //              int box_idx = 0;
   //              for (auto& cropped: croppeds) {
   //                  QString res = QString(m_chars_rec_inferer->run_inf(cropped).c_str());
   //                  qDebug() << "Res result: " << res;
   //                  result_set += res;
   //                  // std::string cropped_name = "Text Box " +std::to_string(++box_idx);
   //                  // cv::imshow(cropped_name, cropped);
   //                  // cv::moveWindow(cropped_name, 100 + box_idx * 50, 100);
   //                  // cv::waitKey(0);
   //              }
   //              qDebug() << "Result set: " << result_set;
   //              m_1_cropped_img->set_inf_result(result_set);
   //
   //              auto end = std::chrono::high_resolution_clock::now();
   //              auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
   //              qDebug() << "推理总耗时： " << duration;
   //  });
   //
   //
   //  connect ( m_camera, &camera_wrapper::img_cropped4inf
   //          , this, [this](QVector<cropped_image>& cropped_images) {
   //              auto start = std::chrono::high_resolution_clock::now();
   //              
   //              int idx = 0;
   //              m_1_cropped_img->set_image(cropped_images[0].image);
   //              for2 = qimage2mat(cropped_images[0].image);
   //              for (auto cropped: cropped_images) {
   //                  idx += 1;
   //                  //qDebug() << "推理结果: " << m_inferer->exec_inf(qimage2mat(cropped.image));  
   //             }
   //              auto end = std::chrono::high_resolution_clock::now();
   //              auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
   //              qDebug() << "推理总耗时： " << duration;
   //       });
   //
   //  m_camera_cropped_layout->setLayout(m_cropped_img_wrapper);

    // m_mainwindow_camera_layout_wrapper->addWidget(m_camera);
    // m_mainwindow_camera_layout_wrapper->setAlignment(m_camera, Qt::AlignLeft | Qt::AlignTop);
    // m_mainwindow_camera_layout_wrapper->setContentsMargins(0, 0, 0, 0);
    // m_mainwindow_camera_layout->setLayout(m_mainwindow_camera_layout_wrapper);
    // m_mainwindow_camera_layout->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //
    // m_camera_rel_layout->addWidget(m_mainwindow_camera_layout);     // camera
    // m_camera_rel_layout->addWidget(m_actions_wrapper);              // actions
    // Sidebar* m_sidebar = new Sidebar();
    // m_mainwindow_layout_wrapper->addWidget(m_sidebar);
    // m_mainwindow_layout_wrapper->addLayout(m_camera_rel_layout);    // (camera + actions) wrapper
    // m_mainwindow_layout_wrapper->addWidget(m_camera_cropped_layout);// camera croppeds
   
        
    m_mainwindow_layout_wrapper->setContentsMargins(0, 0, 0, 0);
    m_mainwindow_layout_wrapper->setSpacing(0);


    m_chars_rec_inferer = new rec_inferer();
    m_chars_det_inferer = new det_inferer();
   
 

    // tool bar and status bar
    m_tool_bar = new tool_bar();
    addToolBar(Qt::TopToolBarArea, m_tool_bar);
    m_status_bar = new status_bar();
    setStatusBar(m_status_bar);
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);
    splitter->setStyleSheet("QSplitter::handle { background: #404040; }");

    connect ( m_tool_bar
            , &tool_bar::send_rtsp_url
            , this, [this](const QString& rtsp_url, const rtsp_config& rtsp_cfg) {
                int MAX_PER_PAGE 
                    =  qobject_cast<grouping_rtsp_stream*>(m_stream_group->widget(0))->get_MAX_PER_PAGE();

                int target_group_index = cam_nums / MAX_PER_PAGE;

                m_stream_group->setCurrentIndex(target_group_index);
                                qDebug() << "\n\n\nUsername: " << rtsp_cfg.username << '\n'
                         << "Password: " << rtsp_cfg.password << '\n'
                         << "IP: " << rtsp_cfg.ip << '\n'
                         << "Port: " << rtsp_cfg.port << '\n'
                         << "Channel: " << rtsp_cfg.channel << '\n'
                         << "Subtype: " << rtsp_cfg.subtype << '\n'
                         << "cam_id: " << cam_nums;
                qDebug() << "MAX_PER_PAGE: " << MAX_PER_PAGE 
                         << ", current group index: " << m_stream_group->currentIndex() 
                         << ", target_group_index: " << target_group_index;

                grouping_rtsp_stream* current_grid 
                    = qobject_cast<grouping_rtsp_stream*>(m_stream_group->currentWidget());
                
                auto cam = current_grid->get_cam_by_id(cam_nums);
                bool success = cam->set_rtsp_stream(rtsp_url);
                //bool success = current_grid->get_cam_by_id(cam_nums)->set_rtsp_stream(rtsp_url); 
                
                if (success) {
                    m_expands_window2rtsp_config[cam_nums] = rtsp_cfg;
                    connect ( cam
                            , &camera_wrapper::img_cropped4inf
                            , this, [this, cam](QVector<cropped_image>& croppeds) {
                                if (!croppeds.empty()) {  
                                    int inf_cam_id = cam->get_cam_id();
                                    cv::Mat cropped = qimage2mat(croppeds[0].image);
                                    m_expanded_window2_inf_cropped[inf_cam_id] = croppeds[0].image;
                                    std::vector<cv::Mat> det_cropped = m_chars_det_inferer->run_inf(cropped);
                                    QString inf_result_set;
                                    for (auto& rec_cropped: det_cropped) 
                                        inf_result_set += m_chars_rec_inferer->run_inf(rec_cropped) + '\t';
                                
                                    m_expanded_window2_inf_res[inf_cam_id] = inf_result_set;
                                    qDebug() << "inf result: " << m_expanded_window2_inf_res[inf_cam_id];
                                }
                             });
                    
                    cam_nums++;
                }
                // update sidebar
                if (m_sidebar) {
                    removeToolBar(m_sidebar);
                    delete m_sidebar;
                }
                
                m_sidebar = new grouping_sidebar( m_stream_group->count()
                                                , this
                                                ) ;
                addToolBar(Qt::LeftToolBarArea, m_sidebar);

                m_sidebar->set_current_group(m_stream_group->currentIndex());
                
                connect ( m_sidebar
                        , &grouping_sidebar::group_selected
                        , this, [this](int group_index) {
                            if (group_index >= 0 && group_index < m_stream_group->count()) 
                                m_stream_group->setCurrentIndex(group_index);
                        });
                
            });
    

    
    m_sidebar = new grouping_sidebar(4);

    m_stream_group = new QStackedWidget();
  

    connect ( m_tool_bar    // bugs: 1. unable to affect all wrappers 2. sidebar activation status updated wrongly
            , &tool_bar::send_scale_factor
            , this, [this](float factor) {
                int MAX_PER_PAGE 
                    = qobject_cast<grouping_rtsp_stream*>(m_stream_group->widget(0))->get_MAX_PER_PAGE();
                
                int group_count = m_stream_group->count();

                for (int group_idx = 0; group_idx < group_count; ++group_idx) {
                    grouping_rtsp_stream* grid = qobject_cast<grouping_rtsp_stream*>(m_stream_group->widget(group_idx));
                    if (!grid) continue;
                    for (int pos = 0; pos < MAX_PER_PAGE; ++pos) {
                        camera_wrapper* cam = grid->get_cam_by_id(pos);

                        if (cam) cam->set_scale_factor(factor);
                    }
                }
            });

    connect ( m_stream_group
            , &QStackedWidget::currentChanged
            , this, [this](int group_index) {
                if (m_sidebar) m_sidebar->set_current_group(group_index);
            });

    int m_total_cams = 50;
    
    const int per_page = grouping_rtsp_stream::get_MAX_PER_PAGE();

    int total_pages = ceil(static_cast<float>(m_total_cams) / per_page);
    
    for(int page = 0; page < total_pages; ++page){
        int startCam = page * per_page;
        bool isLastPage = (page == total_pages-1);
        int currentTotal = isLastPage ? m_total_cams : (startCam + per_page);
        
        auto grid = new grouping_rtsp_stream(startCam,  currentTotal);
        m_stream_group->addWidget(grid);
    }
    
    for (int i = 0; i < m_stream_group->count(); ++i) {
        grouping_rtsp_stream* grid 
            = qobject_cast<grouping_rtsp_stream*>(m_stream_group->widget(i));
        if (grid) {
            connect (grid, &grouping_rtsp_stream::cam_expand_req, this, [this](int cam_id){
                qDebug() << "Requested expand camera id: " << cam_id;
                
                if (m_expanded_windows.contains(cam_id)) {
                    m_expanded_windows[cam_id]->activateWindow();
                    m_expanded_windows[cam_id]->raise();
                    return;
                }

                camera_wrapper* cam = nullptr;
                grouping_rtsp_stream* current_grid 
                    = qobject_cast<grouping_rtsp_stream*>(m_stream_group->currentWidget());

                if (current_grid) {
                    cam = current_grid->get_cam_by_id(cam_id);
                    if (cam) {
                        expanded_camera_window* expanded_window = new expanded_camera_window(cam);
                        //expanded_window->show();
                        expanded_window->setAttribute(Qt::WA_DeleteOnClose, true);
                        m_expanded_windows[cam_id] = expanded_window;
                         

                        connect( expanded_window // -> cropped moving
                               , &QObject::destroyed
                               , this, [this, cam_id](){m_expanded_windows.remove(cam_id); });
                        connect( cam
                               , &camera_wrapper::img_cropped
                               , expanded_window, [expanded_window](QVector<cropped_image>& croppeds) { 
                                    expanded_window->set_cropped_image(croppeds[0].image);  
                               }, Qt::DirectConnection);

                        connect( cam            //  timeout cropped for inference
                               , &camera_wrapper::img_cropped4inf
                               , expanded_window, [this, expanded_window, cam](QVector<cropped_image>& croppeds) {
                                    if (!croppeds.isEmpty()){
                                        int cam_id = cam->get_cam_id();
                                        expanded_window->set_cropped_image(m_expanded_window2_inf_cropped[cam_id]);
                                        expanded_window->set_inf_result(m_expanded_window2_inf_res[cam_id]);
                                    }
                               }, Qt::DirectConnection);


                        rtsp_config rc = m_expands_window2rtsp_config[cam_id];

                        qDebug() << "Send id: " << rc.ip << '\n';

                        expanded_window->show();
                        expanded_window->set_rstp_info(m_expands_window2rtsp_config[cam_id]);
                    } else {
                        qDebug() << "Error: Camera not found for id: " << cam_id;
                    }
                }
            });

        }
    }
    
    connect ( m_sidebar
            , &grouping_sidebar::group_selected
            , this, [this](int group_index) {
                if (group_index >= 0 && group_index < m_stream_group->count()) {
                    m_stream_group->setCurrentIndex(group_index);
                } else {

                }
            });


    splitter->addWidget(m_sidebar);
    splitter->addWidget(m_stream_group);
    
    QList<int> sizes = {
        static_cast<int>(width() * 0.15),
        static_cast<int>(width() * 0.85)
    };
    splitter->setSizes(sizes);

    m_mainwindow_layout_wrapper->addWidget(splitter);

    setWindowTitle("Detector");
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

