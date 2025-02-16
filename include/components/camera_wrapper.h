#pragma once

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <qpixmap.h>
#include <utils/video_capturer.h>


class camera_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit camera_wrapper(QWidget* parent = nullptr);
    ~camera_wrapper();
private slots:
    // void update_frame(QPixmap frame);
private:
    QHBoxLayout* m_camera_layout;
    QLabel* m_video_stream;
    video_capturer* m_video_capturer;
};