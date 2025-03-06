

#include <components/status_bar.h>


status_bar::status_bar(QWidget* parent)
    : QStatusBar{parent}
    {
   m_conn_info = new QLabel("连接信息：");
   addPermanentWidget(m_conn_info); 
}

void status_bar::update_conn_info(const QString& conn_info) {
    m_conn_info->setText(conn_info);
}

void status_bar::showMessage(const QString& msg, int timeout) {

}

void status_bar::updatetime() {
}
