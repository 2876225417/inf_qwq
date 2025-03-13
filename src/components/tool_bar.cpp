

#include "windows/db_config_window.h"
#include "windows/keywords_edit_window.h"
#include "windows/rtsp_config_window.h"
#include "windows/stream_settings_window.h"
#include <components/tool_bar.h>
#include <opencv2/calib3d.hpp>
#include <qaction.h>
#include <qnamespace.h>
#include <qsizepolicy.h>
#include <qwidget.h>

tool_bar::tool_bar(QWidget* parent)
    : QToolBar{"ToolBar", parent}
    {
    QAction* file_act = new QAction("File", this);
    QMenu* file_menu = new QMenu(this);
    file_menu->addAction("new dialog");
    file_menu->addSeparator();
    file_menu->addAction("close");
    file_act->setMenu(file_menu);
    addAction(file_act);

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    addWidget(spacer);

    QAction* add_rtsp_stream_config = new QAction("RTSP", this);
    addAction(add_rtsp_stream_config);

    connect (add_rtsp_stream_config, &QAction::triggered, this, [this]{
        if (!m_rtsp_config_window) {
            m_rtsp_config_window = new rtsp_config_window(this); 
            connect ( m_rtsp_config_window  // -> mainwindow
                    , &rtsp_config_window::send_rtsp_url
                    , this, [this](const QString& rtsp_url, const rtsp_config& rtsp_cfg) {
                        qDebug() << "rtsp url sent already!";
                        qDebug() << "rtsp stream name: " << rtsp_cfg.rtsp_name 
                                 << "Username: " << rtsp_cfg.username << '\n'
                                 << "Password: " << rtsp_cfg.password << '\n'
                                 << "IP: " << rtsp_cfg.ip << '\n'
                                 << "Port: " << rtsp_cfg.port << '\n'
                                 << "Channel: " << rtsp_cfg.channel << '\n'
                                 << "Subtype: " << rtsp_cfg.subtype << '\n'; 
                        emit send_rtsp_url(rtsp_url, rtsp_cfg);
                    }, Qt::QueuedConnection);
        }
        m_rtsp_config_window->show();
        m_rtsp_config_window->raise();
        m_rtsp_config_window->activateWindow();
    });

    QAction* add_stream_settings = new QAction("Setting", this);
    addAction(add_stream_settings);

    connect ( add_stream_settings
            , &QAction::triggered
            , this, [this] {
                if (!m_stream_settings_window) {
                    m_stream_settings_window = new stream_settings_window(this);
                    connect ( m_stream_settings_window
                            , &stream_settings_window::scale_factor_changed
                            , this, [this](float factor){
                                qDebug() << "Scale factor: " << factor;
                                emit send_scale_factor(factor);
                            }, Qt::QueuedConnection);
                }
                m_stream_settings_window->show();
                m_stream_settings_window->raise();
                m_stream_settings_window->activateWindow();
            });
   
    QAction* add_db_config_settings = new QAction("Database", this);
    addAction(add_db_config_settings);

    connect ( add_db_config_settings
            , &QAction::triggered
            , this, [this] {
                if (!m_db_config_window) {
                    m_db_config_window = new db_config_window(this);
                    connect ( m_db_config_window
                            , &db_config_window::connection_established
                            , this, [this](){
                                emit send_database_connected_established();
                            }, Qt::QueuedConnection); 
                 }
                m_db_config_window->show();
                m_db_config_window->raise();
                m_db_config_window->activateWindow();
            });

    QAction* add_keywords_edit_settings = new QAction("Keywords Edit", this);
    addAction(add_keywords_edit_settings);


    connect ( add_keywords_edit_settings
            , &QAction::triggered
            , [this]() {
                if (!m_keywords_edit_window) {   
                    m_keywords_edit_window = new keywords_edit_window(this);
                    connect ( m_keywords_edit_window
                            , &keywords_edit_window::keywords_change
                            , this, [this](const QVector<QString>& keywords){
                                emit send_keywords(keywords);    
                            }, Qt::QueuedConnection);
                }
                m_keywords_edit_window->show();
                m_keywords_edit_window->raise();
                m_keywords_edit_window->activateWindow();

            });

    QAction* add_warning_records_settings = new QAction("Records", this);
    addAction(add_warning_records_settings);

    connect ( add_warning_records_settings
            , &QAction::triggered
            , this, [this]() {
                qDebug() << "Show warning records window";
                if (!m_record_warnings_window) {
                    m_record_warnings_window = new warning_records_window(this);

                    
                }
                m_record_warnings_window->show();
                m_record_warnings_window->raise();
                m_record_warnings_window->activateWindow(); 
            });
    
    setMovable(false);
}
