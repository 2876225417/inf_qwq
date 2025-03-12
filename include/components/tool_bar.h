
#pragma once
#include <QToolBar>
#include <QMenu>
#include <components/tool_bar.h>
#include <windows/rtsp_config_window.h>
#include <windows/stream_settings_window.h>
#include <windows/db_config_window.h>
#include <windows/keywords_edit_window.h>


class tool_bar: public QToolBar{ 
    Q_OBJECT
public:
    explicit tool_bar(QWidget* parent = nullptr);

signals:
    void send_rtsp_url(const QString& rtsp_url, const rtsp_config& rtsp_cfg);
    void send_scale_factor(float factor);

    void send_database_connected_established();
    
    void send_keywords(const QVector<QString>&);
private:
    rtsp_config_window* m_rtsp_config_window = nullptr;

    stream_settings_window* m_stream_settings_window = nullptr;
    db_config_window* m_db_config_window = nullptr;
    keywords_edit_window* m_keywords_edit_window = nullptr;
};
