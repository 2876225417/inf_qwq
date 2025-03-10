
#pragma once
#include <QToolBar>
#include <QMenu>
#include <components/tool_bar.h>
#include <windows/rtsp_config_window.h>


class tool_bar: public QToolBar{ 
    Q_OBJECT
public:
    explicit tool_bar(QWidget* parent = nullptr);
private:
    rtsp_config_window* m_rtsp_config_window = nullptr;
};
