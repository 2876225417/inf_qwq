

#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include <QWidget>
#include <qboxlayout.h>
#include <qtmetamacros.h>
#include <qwidget.h>


class settings_window: public QWidget {
    Q_OBJECT
public:
    explicit settings_window(QWidget* parent = nullptr);
    ~settings_window();

private:
    QVBoxLayout* m_settings_main_layout;


    void setup_UI();
    void setup_connections();

};
#endif
