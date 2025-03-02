#pragma once

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <qpixmap.h>
#include <QGroupBox>


#include <utils/video_capturer.h>

#include <components/draw_overlay.h>

struct cropped_image{
    int number;
    QImage image;
};

class camera_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit camera_wrapper(QWidget* parent = nullptr);
    ~camera_wrapper();


signals:
    void img_cropped(QVector<cropped_image>&);
protected:
    bool eventFilter(QObject*, QEvent*) override;

private slots: 
    QRect rect2coords(const QRect&);
private:
    void rect2image(QVector<rect_data>&);

    QHBoxLayout* m_camera_layout;
    QLabel* m_video_stream;
  
    video_capturer* m_video_capturer;
    draw_overlay* m_draw_overlay;
    QImage m_current_frame;

    QVector<cropped_image> m_cropped_images;
};
