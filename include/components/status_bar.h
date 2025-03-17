

#pragma once

#include <QStatusBar>
#include <QTimer>
#include <QLabel>
#include <QWidget>
#include <QDateTime>
#include <QLayout>

class status_bar: public QStatusBar {
    Q_OBJECT
public:
    explicit status_bar(QWidget* parent = nullptr);
    
    void update_database_conn_status(bool status);
    void update_conn_cnts(int conn_cnts);
public slots:
private:
    QTimer* m_timer;
    QLabel* m_time_stamp;
    
    QLabel* m_conn_cnts;
    QLabel* m_conn_status;
    // database connection status
    // rstp connection count
    // time stamp
    //
};
