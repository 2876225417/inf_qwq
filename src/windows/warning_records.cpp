

#include <qboxlayout.h>
#include <qwidget.h>
#include <windows/warning_records_window.h>


warning_records_window::warning_records_window(QWidget* parent)
    : QWidget{parent, Qt::Window}
    {

    m_main_layout = new QVBoxLayout(this);


    setLayout(m_main_layout);
}


void warning_records_window::setup_UI() {

}

void warning_records_window::setup_connections() {



}
