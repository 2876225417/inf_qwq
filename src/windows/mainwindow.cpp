


#include "components/camera_wrapper.h"
#include "components/cropped_wrapper.h"
#include "components/grouping_rtsp_stream.h"
#include "components/tool_bar.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "utils/chars_ort_inferer.h"
#include "utils/common.h"
#include "utils/common_tools.hpp"
#include "utils/db_manager.h"
#include "utils/db_manager_mini.h"
#include "utils/http_server.h"
#include "utils/ort_inf.h"
#include "utils/ort_inf.hpp"
#include "windows/rtsp_config_window.h"
#include <cmath>
#include <qboxlayout.h>
#include <qcontainerfwd.h>
#include <qgridlayout.h>
#include <qimage.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qsharedpointer.h>
#include <qsplitter.h>
#include <qstackedwidget.h>
#include <qtimer.h>
#include <qwidget.h>
#include <windows/mainwindow.h>
#include <grpc/grpc.h>
#include <components/grouping_sidebar.h>
#include <QStackedWidget>
#include <QSplitter>
#include <windows/expanded_camera_window.h>
#include <QProgressBar>
#include <QProgressDialog>

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
    
    setup_UI();
    setup_connections();
    http_server::instance().fetch_all_rtsp_stream_info();


    QTimer::singleShot(2000, this, [this]() {
        // 延迟自动连接RTSP流
        connect_saved_rtsp_streams();
    });

    setWindowTitle("关键词监视器@春晖");
    resize(1400, 700);  // windows size w: 1400 h: 700
    setCentralWidget(m_mainwindow_layout);
}

mainwindow::~mainwindow() {
       
}

void mainwindow::setup_UI() {
    // Top toolbar  
    m_tool_bar = new tool_bar();
    addToolBar(Qt::TopToolBarArea, m_tool_bar);
    // Bottom status bar
    m_status_bar = new status_bar();
    setStatusBar(m_status_bar);
    // Left sidebar
    m_sidebar = new grouping_sidebar(4);
    addToolBar(Qt::LeftToolBarArea, m_sidebar);
    // RTSP stream group
    m_stream_group = new QStackedWidget();
    int m_total_cams = 48;
    const int per_page = grouping_rtsp_stream::get_MAX_PER_PAGE();
    int total_pages = std::ceil(static_cast<float>(m_total_cams) / per_page);
    
    for (int page = 0; page < total_pages; ++page) {
        int start_cam = page * per_page;
        bool is_last_page = (page == total_pages - 1);
        int current_total = is_last_page ? m_total_cams : (start_cam + per_page);

        auto rtsp_grid = new grouping_rtsp_stream(start_cam, current_total);
        m_stream_group->addWidget(rtsp_grid);
    }

    // Splitter  Sidebar | RTSP stream group
    auto splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);
    splitter->setStyleSheet("QSplitter::handle { background: #404040; }");
    splitter->addWidget(m_sidebar);
    splitter->addWidget(m_stream_group);
    QList<int> sizes{
        static_cast<int>(width() * 0.05f),
        static_cast<int>(width() * 0.95f)
    };
    splitter->setSizes(sizes);

    m_mainwindow_layout_wrapper->addWidget(splitter);
    m_mainwindow_layout->setLayout(m_mainwindow_layout_wrapper);
}


std::tuple<camera_wrapper*, bool>
mainwindow::add_rtsp_stream(const QString& rtsp_url, const rtsp_config& config) {
    int MAX_PER_PAGE
        = qobject_cast<grouping_rtsp_stream*>(m_stream_group->widget(0))
        ->get_MAX_PER_PAGE();

    int target_group_index = cam_nums / MAX_PER_PAGE;

    m_stream_group->setCurrentIndex(target_group_index);

    grouping_rtsp_stream* current_grid
        = qobject_cast<grouping_rtsp_stream*>(m_stream_group->currentWidget());

    auto cam = current_grid->get_cam_by_id(cam_nums);
    cam->get_draw_overlay()->set_rtsp_config(config);

    bool success = cam->set_rtsp_stream(rtsp_url);
    cam->get_draw_overlay()->set_cam_id(config.rtsp_id);
    cam->get_draw_overlay()->set_cam_name(config.rtsp_name);

    cam->get_draw_overlay()->update_cropped_coords( config.cropped_pos.cropped_x
                                                  , config.cropped_pos.cropped_y
                                                  , config.cropped_pos.cropped_dx
                                                  , config.cropped_pos.cropped_dy
                                                  );
                    
    return {cam, success};
}
 
void mainwindow::setup_camera_connections(camera_wrapper* cam, const rtsp_config& config) {
      connect( cam
             , &camera_wrapper::img_cropped4inf
             , [this, cam](QVector<cropped_image>& croppeds){
                    if (!croppeds.empty() && cam->get_draw_overlay()->is_inferrable()) {
                        int inf_cam_id = cam->get_cam_id();
                        cv::Mat cropped = qimage2cvmat(croppeds[0].image);
                        m_expanded_window2_inf_cropped[inf_cam_id] = croppeds[0].image;
                        // m_chars_ort_inferer->run_inf(inf_cam_id, cropped);
                    }
             });

    connect( cam
           , &camera_wrapper::reset_inf_result_after_hint
           , [this](int cam_id) {
                m_expanded_window2_inf_res[cam_id] = "";
           });
            // 显示放大 RTSP 流
            connect( cam
                   , &camera_wrapper::cam_expand_req
                   , [this,cam]() {
                        qDebug() << "Expanding window request received!\n";
                        auto expanded_window = new expanded_camera_window(1, cam);
                        expanded_window->show();
                        expanded_window->activateWindow();
                        expanded_window->raise();

                        connect( cam
                               , &camera_wrapper::img_cropped
                               , [this, expanded_window](QVector<cropped_image>& croppeds) {
                                    if (!croppeds.empty()) expanded_window->set_cropped_image(croppeds[0].image);
                               });

                         connect( cam
                                , &camera_wrapper::img_cropped4inf
                                , [this, expanded_window](QVector<cropped_image>& croppeds) {
                                    if (!croppeds.empty()) expanded_window->set_cropped_image(croppeds[0].image);
                                });
                    });
                    
            // 配置关键字
            connect( m_tool_bar
                   , &tool_bar::send_keywords
                   , [this, cam](const QVector<QString>& keywords) {
                        int cam_id = cam->get_cam_id();
                        qDebug() << "cam_id: " << cam_id << "with keywords: ";
                        cam->set_do_keywords(keywords);
                        for(auto& keyword: keywords) qDebug() << keyword << ' ';
                    });
                   
            // 配置 HTTP 警报
            connect( m_tool_bar
                   , &tool_bar::send_http_url
                   , [this, cam](const QString& http_url, bool radiated) {
                        qDebug() << "Received here";
                        radiated = true;
                        cam->get_draw_overlay()->set_http_url(http_url);
                        cam->get_draw_overlay()->set_http_url_status(radiated);
                        qDebug() << "Received http url: " << http_url 
                                 << " whether enable radiated all: " << (radiated ? "Enabled" : "Disabled");
                   });

}


void mainwindow::update_sidebar() {
    if (m_sidebar) {
        removeToolBar(m_sidebar);
        delete  m_sidebar;
    }

    m_sidebar = new grouping_sidebar(m_stream_group->count(), this);
    addToolBar(Qt::LeftToolBarArea, m_sidebar);
                
    // 
    m_sidebar->set_current_group(m_stream_group->currentIndex());
    connect( m_sidebar
           , &grouping_sidebar::group_selected
           , [this](int group_index) {
                if (group_index >= 0 && group_index < m_stream_group->count())
                    m_stream_group->setCurrentIndex(group_index);
           });  
}

void mainwindow::connect_saved_rtsp_streams() {
    if (m_rtsp_configs.isEmpty()) {
        qDebug() << "No saved RTSP streams to connect.";
        return;
    }

    qDebug() << "Connecting to" << m_rtsp_configs.size() << "saved rtsp configs...";
    
    for (const auto& config: m_rtsp_configs) {
        QString rtsp_url = config.rtsp_url;
        qDebug() << "Connecting to: " << config.rtsp_name << "-" << rtsp_url;
        
        auto [cam, success] = add_rtsp_stream(rtsp_url, config);

        if (success) {
            qDebug() << "Successfully connected to: " << config.rtsp_name;
            m_expands_window2rtsp_config[cam_nums] = config;
            setup_camera_connections(cam, config);
            emit conn_cnt_changed(cam_nums++);
            m_status_bar->update_conn_cnts(cam_nums);
        } else {
            qDebug() << "Failed to connect to: " <<  config.rtsp_name;
        }
    }
    update_sidebar();
}


void mainwindow::setup_connections() {
    connect( m_tool_bar
           , &tool_bar::send_rtsp_url
           , [this](const QString& rtsp_url, const rtsp_config& rtsp_config) {
                auto [cam, success] = add_rtsp_stream(rtsp_url, rtsp_config);               
                // 对连接成功的 RTSP 窗口进行信号连接
                if (success) {
                    QString rtsp_protocal_type  = rtsp_config.is_hk() ? "HKVISION" : "DAHUA";
                    QString user_name           = rtsp_config.username;
                    QString rtsp_ip             = rtsp_config.ip;
                    int port                    = rtsp_config.ip.toInt();
                    QString channel             = rtsp_config.channel;
                    QString subtype             = rtsp_config.subtype;
                    QString url                 = rtsp_url; 
                    
                    http_server::instance().add_rtsp_source(
                        rtsp_protocal_type,
                        user_name, 
                        rtsp_ip,
                        port,
                        channel, 
                        subtype,
                        url
                    );
                    
                    qDebug() << "|-----Connection Info-----|" << '\n'
                             << "RTSP Protocal Type: " << rtsp_protocal_type << '\n'
                             << "Username: "           << user_name          << '\n'
                             << "RTSP_ip:  "           << rtsp_ip            << '\n'
                             << "Port:     "           << port               << '\n'
                             << "Channel:  "           << channel            << '\n'
                             << "Subtype:  "           << subtype            << '\n'
                             << "URL:      "           << url                << '\n';
                  
                    m_expands_window2rtsp_config[cam_nums] = rtsp_config;
                    setup_camera_connections(cam, rtsp_config);
                    // 将截取的图片进行推理
                    emit conn_cnt_changed(cam_nums++);
                    m_status_bar->update_conn_cnts(cam_nums);
                }

            update_sidebar();

          });

    // 切换布局
    connect( m_tool_bar
           , &tool_bar::send_rtsp_stream_num
           , [this](int rtsp_count) {
                qDebug() << "rtsp_count: " << rtsp_count;
                QSettings settings("ChunHui", "rtsp_monitor");
                settings.setValue("layout/grid_size", rtsp_count);
                
            QMessageBox::information(this, "布局更改", QString("视频网格布局已更改 %1x%1, \n请重新启动应用以应用新的布局设置").arg(static_cast<int>(std::sqrt(rtsp_count))), QMessageBox::Ok);
           });
    
    // 切换分组
    connect( m_stream_group, &QStackedWidget::currentChanged, [this](int group_index) {
        if (m_sidebar) m_sidebar->set_current_group(group_index);
    });

    // 启动时获取所有 RTSP 信息 
    connect( &http_server::instance()
           , &http_server::send_all_rtsp_stream_info
           , [this](const QVector<rtsp_config>& rtsp_configs) {
                m_rtsp_configs = rtsp_configs;
                qDebug() << "Fetched all in mainw and m_rtsp_configs' size: " << m_rtsp_configs.size(); 
                for (const auto& rtsp_config: rtsp_configs) qDebug() << rtsp_config;
           });
}

