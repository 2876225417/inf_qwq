#include "components/camera_wrapper.h"
#include "components/cropped_wrapper.h"
#include "components/grouping_rtsp_stream.h"
#include "components/tool_bar.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "utils/chars_ort_inferer.h"
#include "utils/db_manager.h"
#include "utils/db_manager_mini.h"
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
    m_mainwindow_layout_wrapper->setContentsMargins(0, 0, 0, 0);
    m_mainwindow_layout_wrapper->setSpacing(0);


    // initialize database when launch
    
    db_manager::instance().database_initialize();

    m_chars_rec_inferer = new rec_inferer();
    m_chars_det_inferer = new det_inferer();
   
    // tool bar and status bar
    m_tool_bar = new tool_bar();
    addToolBar(Qt::TopToolBarArea, m_tool_bar);
    m_status_bar = new status_bar();
    setStatusBar(m_status_bar);
    
    m_chars_ort_inferer = new chars_ort_inferer();

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);
    splitter->setStyleSheet("QSplitter::handle { background: #404040; }");

    connect ( m_tool_bar
            , &tool_bar::send_database_connected_established
            , this, [this]() {
                qDebug() << "Connection established";

            });

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
                cam->get_draw_overlay()->set_rtsp_config(rtsp_cfg);

                bool success = cam->set_rtsp_stream(rtsp_url);
                //bool success = current_grid->get_cam_by_id(cam_nums)->set_rtsp_stream(rtsp_url); 
                cam->get_draw_overlay()->set_cam_id(cam->get_cam_id() + 1);
                cam->get_draw_overlay()->set_cam_name(rtsp_cfg.rtsp_name);
                if (success) {
                    m_expands_window2rtsp_config[cam_nums] = rtsp_cfg;
                    connect ( cam
                            , &camera_wrapper::img_cropped4inf
                            , this, [this, cam](QVector<cropped_image>& croppeds) {
                                if (!croppeds.empty() && cam->get_draw_overlay()->is_inferrable()) {  
                                    int inf_cam_id = cam->get_cam_id();
                                    cv::Mat cropped = qimage2mat(croppeds[0].image);
                                    m_expanded_window2_inf_cropped[inf_cam_id] = croppeds[0].image;
                                    m_chars_ort_inferer->run_inf(inf_cam_id, cropped); // aysnc inf
                                }
                             });

                    connect ( cam
                            , &camera_wrapper::reset_inf_result_after_hint
                            , this, [this](int cam_id) { m_expanded_window2_inf_res[cam_id] = ""; });       
             
                    connect ( cam
                            , &camera_wrapper::cam_expand_req
                            , this, [this, cam]() {
                                qDebug() << "expanding window request received!";

                                expanded_camera_window* expanded_window = new expanded_camera_window(cam);
                                expanded_window->show();
                                expanded_window->activateWindow();
                                expanded_window->raise();

                                connect ( cam
                                        , &camera_wrapper::img_cropped
                                        , this, [this, expanded_window](QVector<cropped_image>& croppeds) {
                                            if (!croppeds.empty())
                                                expanded_window->set_cropped_image(croppeds[0].image);
                                        });

                                connect ( cam
                                        , &camera_wrapper::img_cropped4inf
                                        , this, [this, expanded_window](QVector<cropped_image>& croppeds) {
                                            if (!croppeds.empty()) {
                                                expanded_window->set_cropped_image(croppeds[0].image);
                                            } 
                                        });
                    });

                    connect ( m_tool_bar
                            , &tool_bar::send_keywords
                            , this, [this, cam](const QVector<QString>& keywords){ 
                                int cam_id = cam->get_cam_id();
                                qDebug() << "cam_id: " << cam_id << "with keyword: ";
                                cam->set_do_keywords(keywords);
                                for(auto& keyword: keywords) qDebug() << keyword << ' ';
                            });

                    connect ( m_tool_bar
                            , &tool_bar::send_scale_factor
                            , this, [this, cam](float factor) {
                               cam->set_scale_factor(factor); 
                            });

                    connect ( m_tool_bar        // send http url
                            , &tool_bar::send_http_url
                            , this, [this, cam](const QString& http_url, bool radiated){
                                qDebug() << "Received here";
                                cam->get_draw_overlay()->set_http_url(http_url);
                                cam->get_draw_overlay()->set_http_url_status(radiated);
                                qDebug() << "Received http url: " << http_url 
                                         << " whether enable radiated all: " << (radiated ? "Enabled" : "Disabled"); 
                                
                            });

                

                    connect ( m_chars_ort_inferer
                            , &chars_ort_inferer::inference_completed
                            , this, [this, cam](int cam_id, const std::vector<std::string> results) {
                                QString result_set;
                                for (const auto& result: results) {
                                    result_set += result;
                                }
                                // update camera wrapper inf result
                                cam->invoke();
                                qDebug() << "cam id: " << cam_id;
                                m_expanded_window2_inf_res[cam_id] = result_set;

                                for (int i = 0; i < m_expanded_window2_inf_res.size(); i++) {
                                    if (i == cam->get_cam_id() && !m_expanded_window2_inf_res[i].isEmpty()) {
                                        cam->set_do_inf_result(m_expanded_window2_inf_res[i]);
                                        
                                        qDebug() << "Inf result: " << m_expanded_window2_inf_res;
                                        cam->get_draw_overlay()->update_keywords_no_args();
                                        cam->get_draw_overlay()->set_status();
                                        int record_id = cam->get_draw_overlay()->record_warning2db();
                                        cam->get_draw_overlay()->set_last_record_id(record_id);
                                        cam->get_draw_overlay()->hint_warning();

                                        if (db_manager::instance().is_connected()) {
                                            db_manager::instance().add_inf_result(cam_id, "keywords", result_set);
                                        }
                                    }
                                }
                                // should clear the coords before detect again 
                                //cam->set_do_inf_result(m_expanded_window2_inf_res[cam_id]);
                            });
                    


                    emit conn_cnt_changed(cam_nums++);
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
  
    connect ( m_stream_group
            , &QStackedWidget::currentChanged
            , this, [this](int group_index) {
                if (m_sidebar) m_sidebar->set_current_group(group_index);
            });

  

    int m_total_cams = 48;
    
    const int per_page = grouping_rtsp_stream::get_MAX_PER_PAGE();

    int total_pages = ceil(static_cast<float>(m_total_cams) / per_page);
    
    for(int page = 0; page < total_pages; ++page){
        int startCam = page * per_page;
        bool isLastPage = (page == total_pages-1);
        int currentTotal = isLastPage ? m_total_cams : (startCam + per_page);
        
        auto grid = new grouping_rtsp_stream(startCam,  currentTotal);

       m_stream_group->addWidget(grid);
    }
    
    // for (int i = 0; i < m_stream_group->count(); ++i) {
    //     grouping_rtsp_stream* grid 
    //         = qobject_cast<grouping_rtsp_stream*>(m_stream_group->widget(i));
    //     if (grid) {
    //         connect (grid, &grouping_rtsp_stream::cam_expand_req, this, [this](int cam_id){
    //             qDebug() << "Requested expand camera id: " << cam_id;
    //             
    //             if (m_expanded_windows.contains(cam_id)) {
    //                 m_expanded_windows[cam_id]->activateWindow();
    //                 m_expanded_windows[cam_id]->raise();
    //                 return;
    //             }
    //
    //             camera_wrapper* cam = nullptr;
    //             grouping_rtsp_stream* current_grid 
    //                 = qobject_cast<grouping_rtsp_stream*>(m_stream_group->currentWidget());
    //
    //             if (current_grid) {
    //                 cam = current_grid->get_cam_by_id(cam_id);
    //                 if (cam) {
    //                     expanded_camera_window* expanded_window = new expanded_camera_window(cam);
    //                     //expanded_window->show();
    //                     expanded_window->setAttribute(Qt::WA_DeleteOnClose, true);
    //                     m_expanded_windows[cam_id] = expanded_window;
    //                      
    //
    //                     connect( expanded_window // -> cropped moving
    //                            , &QObject::destroyed
    //                            , this, [this, cam_id](){m_expanded_windows.remove(cam_id); });
    //                     
    //                     
    //                     connect ( expanded_window
    //                             , &expanded_camera_window::keywords_changed
    //                             , this, [this, cam_id](const QString& keywords) {
    //                                 m_expanded_window2_keywords[cam_id] = keywords;   
    //                                 qDebug() << "Received keywords: " << m_expanded_window2_keywords;
    //                             });
    //
    //                     connect( cam
    //                            , &camera_wrapper::img_cropped
    //                            , expanded_window, [expanded_window](QVector<cropped_image>& croppeds) { 
    //                                 expanded_window->set_cropped_image(croppeds[0].image);  
    //                            }, Qt::DirectConnection);
    //
    //                     connect( cam            //  timeout cropped for inference
    //                            , &camera_wrapper::img_cropped4inf
    //                            , expanded_window, [this, expanded_window, cam_id](QVector<cropped_image>& croppeds) {
    //                                 if (!croppeds.isEmpty()){
    //                                     expanded_window->set_cropped_image(m_expanded_window2_inf_cropped[cam_id]);
    //                                     expanded_window->set_inf_result(m_expanded_window2_inf_res[cam_id]);
    //                                 }
    //                            }, Qt::DirectConnection);
    //
    //
    //                     rtsp_config rc = m_expands_window2rtsp_config[cam_id];
    //
    //                     qDebug() << "Send id: " << rc.ip << '\n';
    //
    //                     expanded_window->show();
    //                     expanded_window->set_rstp_info(m_expands_window2rtsp_config[cam_id]);
    //                 } else {
    //                     qDebug() << "Error: Camera not found for id: " << cam_id;
    //                 }
    //             }
    //         });
    //
    //     }
    // }
    
    

    
    splitter->addWidget(m_sidebar);
    splitter->addWidget(m_stream_group);
    
    QList<int> sizes = {
        static_cast<int>(width() * 0.15),
        static_cast<int>(width() * 0.85)
    };
    splitter->setSizes(sizes);

    m_mainwindow_layout_wrapper->addWidget(splitter);

    setWindowTitle("关键词监视器@春晖");
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


// bool mainwindow::initialize_database() {
//     QSettings settings("Chun Hui", "inf_qwq");
//     QString host = settings.value("Database/Host", "localhost").toString();
//     QString db_name = settings.value("Database/Database", "det_qwq").toString();
//     QString user = settings.value("Database/Usernaem", "postgres").toString();
//     QString password = settings.value("Database/Password", "20041025").toString();
//     int port = settings.value("Database/Port", 5432).toInt();
//
//     bool success = db_manager::instance().initialize( "QPSQL", host
//                                                     , db_name, user
//                                                     , password, port
//                                                     );
//     if (success) qDebug() << "Database connection intialized successfully!";
//     else qWarning() << "Failed to intialize database connection";
//
//     return success;
// }
