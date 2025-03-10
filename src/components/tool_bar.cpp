

#include "windows/rtsp_config_window.h"
#include <components/tool_bar.h>
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
        if (!m_rtsp_config_window) { m_rtsp_config_window = new rtsp_config_window(this); }
        m_rtsp_config_window->show();
        m_rtsp_config_window->raise();
        m_rtsp_config_window->activateWindow();
    });

    setMovable(false);
}
