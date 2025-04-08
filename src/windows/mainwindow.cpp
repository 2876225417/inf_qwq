


#include "components/camera_wrapper.h"
#include "components/cropped_wrapper.h"
#include "components/grouping_rtsp_stream.h"
#include "components/tool_bar.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "utils/chars_ort_inferer.h"
#include "utils/common_tools.hpp"
#include "utils/db_manager.h"
#include "utils/db_manager_mini.h"
#include "utils/http_server.h"
#include "utils/ort_inf.h"
#include "utils/ort_inf.hpp"
#include "windows/rtsp_config_window.h"
#include <cmath>
#include <qboxlayout.h>
#include <qgridlayout.h>
#include <qimage.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qsharedpointer.h>
#include <qsplitter.h>
#include <qstackedwidget.h>
#include <qwidget.h>
#include <windows/mainwindow.h>
#include <grpc/grpc.h>

#include <components/grouping_sidebar.h>
#include <QStackedWidget>




#include <QSplitter>
#include <windows/expanded_camera_window.h>

/* ---- mainwindow layout ---- */
mainwindow::mainwindow(QWidget* parent)
    : QMainWindow{parent}
    , m_mainwindow_layout{new QWidget()}
    , m_mainwindow_layout_wrapper{new QHBoxLayout()}
    {     

    QSettings settings("ChunHui", "rtsp_monitor");
    int saved_grid_size = settings.value("layout/grid_size", 4).toInt();

    grouping_rtsp_stream::set_MAX_PER_PAGE(saved_grid_size);

    m_mainwindow_layout_wrapper->setContentsMargins(0, 0, 0, 0);
    m_mainwindow_layout_wrapper->setSpacing(0);
    
    m_http_server = new http_server(this);

    // tool bar and status bar
    m_tool_bar = new tool_bar();
    addToolBar(Qt::TopToolBarArea, m_tool_bar);
    m_status_bar = new status_bar();
    setStatusBar(m_status_bar);

    connect ( &db_manager::instance()
            , &db_manager::database_conn_status
            , this, [this](bool status) {
               m_status_bar->update_database_conn_status(status); 
            });
    db_manager::instance().database_initialize();
 
    m_chars_ort_inferer = new chars_ort_inferer();

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

                grouping_rtsp_stream* current_grid 
                    = qobject_cast<grouping_rtsp_stream*>(m_stream_group->currentWidget());
                
                auto cam = current_grid->get_cam_by_id(cam_nums);
                cam->get_draw_overlay()->set_rtsp_config(rtsp_cfg);

                bool success = cam->set_rtsp_stream(rtsp_url);
                cam->get_draw_overlay()->set_cam_id(cam->get_cam_id() + 1);
                cam->get_draw_overlay()->set_cam_name(rtsp_cfg.rtsp_name);
                
                if (success) {
                    // 添加连接 rtsp 信息 
                    db_manager::instance().add_rtsp_config( rtsp_cfg.username
                                                          , rtsp_cfg.ip
                                                          , rtsp_cfg.port
                                                          , rtsp_cfg.channel
                                                          , rtsp_cfg.subtype
                                                          , rtsp_url
                                                          );
                    m_http_server->add_rtsp_source("123", "123", "123", 123, "123", "123", "123");

                    m_expands_window2rtsp_config[cam_nums] = rtsp_cfg;
                    connect ( cam
                            , &camera_wrapper::img_cropped4inf
                            , this, [this, cam](QVector<cropped_image>& croppeds) {
                                if (!croppeds.empty() && cam->get_draw_overlay()->is_inferrable()) {  
                                    int inf_cam_id = cam->get_cam_id();
                                    cv::Mat cropped = qimage2cvmat(croppeds[0].image);
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

                                expanded_camera_window* expanded_window = new expanded_camera_window(1, cam);
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
                                // force enabled
                                radiated = true;
                                cam->get_draw_overlay()->set_http_url(http_url);
                                cam->get_draw_overlay()->set_http_url_status(radiated);
                                qDebug() << "Received http url: " << http_url 
                                         << " whether enable radiated all: " << (radiated ? "Enabled" : "Disabled"); 
                            });




                                   emit conn_cnt_changed(cam_nums++);
                    m_status_bar->update_conn_cnts(cam_nums);
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
    


    connect ( m_tool_bar
            , &tool_bar::send_rtsp_stream_num
            , this, [this](int rtsp_count) {
                qDebug() << "rtsp_count: " << rtsp_count;
                QSettings settings("ChunHui", "rtsp_monitor");
                settings.setValue("layout/grid_size",rtsp_count);
            
                QMessageBox::information(this, "布局更改", QString("视频网格布局已更改 %1x%1, \n请重新启动应用程序以应用新的布局设置").arg(static_cast<int>(std::sqrt(rtsp_count))), QMessageBox::Ok);
             
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
    auto splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);
    splitter->setStyleSheet("QSplitter::handle { background: #404040; }");

  
    splitter->addWidget(m_sidebar);
    splitter->addWidget(m_stream_group);
    
    QList<int> sizes{
        static_cast<int>(width() * 0.05f), // group sidebar -> 0.05
        static_cast<int>(width() * 0.95f)  // group stream  -> 0.95
    };

    splitter->setSizes(sizes);

    m_mainwindow_layout_wrapper->addWidget(splitter);

    m_mainwindow_layout->setLayout(m_mainwindow_layout_wrapper);    // mainwindow  

    setWindowTitle("关键词监视器@春晖");
    resize(1400, 700);  // windows size w: 1400 h: 700
    setCentralWidget(m_mainwindow_layout);
}

mainwindow::~mainwindow() {
       
}

void mainwindow::setup_UI() {

}

void mainwindow::setup_connections() {

}


