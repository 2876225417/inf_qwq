

#include <components/status_bar.h>
#include <qdatetime.h>
#include <qlabel.h>


status_bar::status_bar(QWidget* parent)
    : QStatusBar{parent}
    {
    m_conn_cnts = new QLabel("连接数: 无RTSP流连接");
    m_time_stamp = new QLabel();
    m_conn_status = new QLabel();

    m_timer = new QTimer(this);
    connect ( m_timer
            , &QTimer::timeout
            , this, [this]() {
                QDateTime current = QDateTime::currentDateTime();
                QString time_str = current.toString("yyyy-MM-dd hh:mm:ss");
                m_time_stamp->setText(time_str);
            }); m_timer->start(1000);
        
    addPermanentWidget(m_conn_status);
    addPermanentWidget(m_conn_cnts);
    addWidget(m_time_stamp);
}

void status_bar::update_database_conn_status(bool status) {
    if (status) {
        m_conn_status->setText(tr("数据库连接状态: 已连接 | "));
        m_conn_status->setStyleSheet("QLabel {color: green; }");
    } else {
        m_conn_status->setText(tr("数据库连接状态: 未连接 | "));
        m_conn_status->setStyleSheet("QLabel { color: red; }");
    }
}

void status_bar::update_conn_cnts(int conn_cnts) {
    m_conn_cnts->setText(QString("连接数: %1").arg(conn_cnts));
}



