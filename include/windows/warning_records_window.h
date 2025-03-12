

#ifndef WARNING_RECORDS_WINDOW_H
#define WARNING_RECORDS_WINDOW_H

#include <QWidget>
#include <qboxlayout.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qwidget.h>



class warning_records_window: public QWidget {
    Q_OBJECT
public:
    warning_records_window(QWidget* parent = nullptr);


private:
    QVBoxLayout* m_main_layout;


    void setup_UI();
    void setup_connections();


};
#endif
