


#include "windows/stream_settings_window.h"
#include <qboxlayout.h>
#include <qwidget.h>
#include <windows/settings_window.h>


settings_window::settings_window(QWidget* parent)
    : QWidget{parent, Qt::Window}
    {
    setWindowTitle("Settings");
    setup_UI();
    setLayout(m_settings_main_layout);
}

settings_window::~settings_window() {}

void settings_window::setup_UI() {
    m_settings_main_layout = new QVBoxLayout(this);
    m_settings_main_layout->setSpacing(10);
    m_settings_main_layout->setContentsMargins(10, 10, 10, 10);

}



void settings_window::setup_connections() {
    
}
