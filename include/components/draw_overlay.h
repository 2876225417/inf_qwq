


#ifndef DRAW_OVERLAY_H
#define DRAW_OVERLAY_H


#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVector>
#include <QComboBox>
#include <QDebug>
#include <qcombobox.h>
#include <qevent.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>

#include <windows/rtsp_config_window.h>


struct rect_data {
    QRect rect;
    int number =1;
};

class QTimer;


class expanded_camera_window;

class draw_overlay: public QWidget {
    Q_OBJECT
public:
    explicit draw_overlay(int cam_id, QWidget* parent = nullptr);
    void resize(const QSize& size) { 
        QWidget::resize(size);
        update();
    }
    
    bool start_crop_image4inf();
    
    void update_inf_result();
    void set_inference_result(const QString& result);
    void set_keywords(const QVector<QString>& keywords);
    void set_status();
    void set_cam_name(const QString& cam_name);
    
    void update_keywords_no_args();
    void set_cam_id(int cam_id) { m_cam_id = cam_id; }
    void set_rtsp_config(const rtsp_config& rtsp_cfg) { m_rtsp_config = rtsp_cfg; }

    void hint_warning();    // warning relatives
    int record_warning2db();   
   
    bool is_inferrable() { return m_is_inf; } 
    
    void set_current_rtsp_url(const QString& rtsp_url) { m_current_rtsp_url = rtsp_url; }
    void set_http_url(const QString& url) { m_http_url = url; qDebug() << "update url: " << m_http_url;}
    void set_http_url_status(bool radiated) { m_enable_http_url = radiated; }
signals:
    void selected(QVector<rect_data>& rects);
    void update_frame_moving(QVector<rect_data>& rects);
    void timer_timeout_update(QVector<rect_data>& rects);
    void expand_camera_request(const QString& rtsp_url);


    void close_conn();

    void reset_inf_result_after_hint(int cam_id);

    void change_cam_rtsp_configs();

    // rtsp configs signals
    void suspend_cam();
    void resume_cam();
    void switch_cam(const QString& rtsp_url, const rtsp_config& rtsp_cfg);
     
    // expanded window
    //void set_expanded_window_cropped_img(const QImage& cropped);  
private:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    bool eventFilter(QObject* obj, QEvent* e) override;
    bool event(QEvent* e) override;
    void leaveEvent(QEvent* event) override;

    bool is_allerted() const { return m_keywords.isEmpty(); }
    
    static QString detect_matched_keywords( const QString& text
                                          , const QVector<QString>& all_keywords);

public:
    enum resize_handle {
        none = 0,
        top_left,
        top_right,
        bottom_left,
        bottom_right,
        top,
        right,
        bottom,
        left
    };
    
    // enable http_url msg
    QString m_http_url;
    bool m_enable_http_url;

    QNetworkAccessManager* m_network_mgr;
    void send_http_alarm(int record_id);
    void handle_http_response(QNetworkReply* reply);
    
    void set_last_record_id(int record_id) { m_last_record_id = record_id; }

    int m_last_record_id;

    bool m_is_inf = true;
    
    int m_cam_id;
    QVector<rect_data> m_rects;
    QPoint m_start;
    QRect m_current_rect;
    bool m_is_dragging = false;
    int m_hover_idx = -1;
    int m_drag_idx = -1;
    QPoint m_drag_start_pos;
    QComboBox* m_number_combobox;
    int m_edit_idx = -1;
    QTimer* m_timer;
    bool m_expand_btn_hovered = false;
    bool m_switch_btn_hovered = false;


    resize_handle m_resize_handle = none;
    int m_resize_idx = -1;
    const int m_handle_size = 8;


    QVector<QString> m_all_keywords;
    QString m_inference_result; // 2db
    QString m_keywords; // 2db
    bool m_is_normal_status = true; // 2db
    int cam_id; // 2db
    rtsp_config m_rtsp_config; // 2db
    QString m_cam_name;

    void draw_status_indicator(QPainter& painter);
    void draw_inference_result(QPainter& painter);
    void draw_camera_id(QPainter& painter);
    void draw_camera_name(QPainter& painter);

    bool m_show_warning = false;
    QTimer* m_warning_timer = nullptr;
    void draw_warning(QPainter& painter);

    void update_hover_state(const QPoint& pos);
    QRect paint_close_btn(const QRect& rect) const;
    QRect get_expand_btn_rect() const;
    QRect get_switch_btn_rect() const;

    void handle_rect_number_changed(int idx);
    
    void check_resize_handles(const QPoint& pos);
    QRect get_resize_handle_rect(const QRect& rect, resize_handle handle) const;
    void update_rect_with_resize(const QPoint& pos);
    
    void draw_resize_handles(QPainter& painter, const QRect& rect);

    rtsp_config_window* m_rtsp_config_window = nullptr; 
    QString m_current_rtsp_url;
};

#endif
