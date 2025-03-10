

#include "windows/rtsp_config_window.h"
#include "windows/stream_settings_window.h"
#include <components/tool_bar.h>
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

    QAction* add_rtsp_stream_config = new QAction("+", this);
    addAction(add_rtsp_stream_config);

    connect (add_rtsp_stream_config, &QAction::triggered, this, [this]{
        if (!m_rtsp_config_window) {
            m_rtsp_config_window = new rtsp_config_window(this); 
            connect ( m_rtsp_config_window
                    , &rtsp_config_window::send_rtsp_url
                    , this, [this](const QString& rtsp_url) {
                        qDebug() << "rtsp url sent already!";
                        emit send_rtsp_url(rtsp_url);
                    }, Qt::QueuedConnection);
        }
        m_rtsp_config_window->show();
        m_rtsp_config_window->raise();
        m_rtsp_config_window->activateWindow();
    });

    QAction* add_stream_settings = new QAction("-", this);
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

    
    setMovable(false);
}
