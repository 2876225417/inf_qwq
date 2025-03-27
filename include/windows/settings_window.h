

#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include <QWidget>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <QGroupBox>
#include <QLabel>

class settings_window: public QWidget {
    Q_OBJECT
public:
    explicit settings_window(QWidget* parent = nullptr);
    ~settings_window();


signals:
    void send_rtsp_stream_count_changed(int rtsp_count);

private:
    QVBoxLayout* m_settings_main_layout;
    
    

    // Adjust RTSP Stream Count
    QHBoxLayout* m_adjust_rtsp_stream_count_layout;
    QLabel* m_rtsp_stream_count_label;
    QComboBox* m_adjust_rtsp_stream_count_combo;


    void setup_UI();
    void setup_connections();

};
#endif
