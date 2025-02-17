#pragma once

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <qpixmap.h>
#include <QGroupBox>


#include <utils/video_capturer.h>
#include <components/draw_overlay.h>

class camera_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit camera_wrapper(QWidget* parent = nullptr);
    ~camera_wrapper();


signals:
    void img_cropped(QImage&);
protected:
    bool eventFilter(QObject*, QEvent*) override;

private slots: 
    QRect rect2coords(const QRect&);
private:
    QHBoxLayout* m_camera_layout;
    QLabel* m_video_stream;
  
    video_capturer* m_video_capturer;
    draw_overlay* m_draw_overlay;
    QImage m_current_frame;

};