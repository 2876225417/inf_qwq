#pragma once

#include "utils/ort_inf.h"
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <qpixmap.h>
#include <QGroupBox>


#include <utils/video_capturer.h>

#include <components/draw_overlay.h>

#include <utils/chars_ort_inferer.h>

struct cropped_image{
    int number;
    QImage image;
};

class camera_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit camera_wrapper(int cam_id, QWidget* parent = nullptr);
    ~camera_wrapper();
    bool set_rtsp_stream(const QString& rtsp_url);
    void set_scale_factor(double factor);
    int get_cam_id() const;
    QString get_rtsp_url() const;
    QString get_rtsp_config() const;

    void invoke() { qDebug() << "Invoked! and cam_id: " << m_cam_id;}
    void set_do_inf_result(const QString& inf_res) { m_draw_overlay->set_inference_result(inf_res); }
    void set_do_keywords(const QVector<QString>& keywords) { m_draw_overlay->set_keywords(keywords); }
    
    draw_overlay* get_draw_overlay() { return m_draw_overlay; }
    
    void update_keywords();     // again !!!
signals:
    // send cropped img when selected and moving
    void img_cropped(QVector<cropped_image>&);
    // send cropped img when start inferring
    void img_cropped4inf(QVector<cropped_image>&);
    // send cameram expanding request
    void cam_expand_req(int cam_id);
    // send inf result
    void inf_result(const QString& inf_result);

protected:
    bool eventFilter(QObject*, QEvent*) override;
private slots: 
    QRect rect2coords(const QRect&);
private:
    void rect2image(QVector<rect_data>&);
    void start_inf();
    cv::Mat qimage2mat(QImage& qimage);

     
    QHBoxLayout* m_camera_layout;
    QLabel* m_video_stream;
  
    video_capturer* m_video_capturer;
    draw_overlay* m_draw_overlay;
    QImage m_current_frame;

    QVector<cropped_image> m_cropped_images;

    QTimer* m_timer;
    
    int m_cam_id;

    //chars_ort_inferer* m_chars_inferer;
};
