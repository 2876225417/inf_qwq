

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

public slots:
    void showMessage(const QString& msg, int timeout = 0);
    void updatetime();
private:
    QTimer* m_timer;
    QLabel* m_time_label;

};