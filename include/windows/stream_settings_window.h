

#ifndef STREAM_SETTINGS_WINDOW_H
#define STREAM_SETTINGS_WINDOW_H

#include <QWidget>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qtmetamacros.h>

#include <QLabel>

class stream_settings_window: public QWidget{ 
    Q_OBJECT
public:
    explicit stream_settings_window(QWidget* parent = nullptr);

signals:
    void scale_factor_changed(float factor);
private:
    // stream setting options layout
    QVBoxLayout* m_stream_settings_main_layout;
    
    // adjust scale factor 
    QHBoxLayout* m_adjust_scale_factor_wrapper;
    QLabel* m_adjust_scale_factor_label;
    QComboBox*   m_adjust_scale_factor;


};


#endif  // STREAM_SETTINGS_WINDOW_H
