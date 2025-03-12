


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


struct rect_data {
    QRect rect;
    int number =1;
};

class QTimer;

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
    
    void update_keywords_no_args();
signals:
    void selected(QVector<rect_data>& rects);
    void update_frame_moving(QVector<rect_data>& rects);
    void timer_timeout_update(QVector<rect_data>& rects);
    void expand_camera_request(int cam_id);
private:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    bool eventFilter(QObject* obj, QEvent* e) override;
    bool event(QEvent* e) override;
   
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

    resize_handle m_resize_handle = none;
    int m_resize_idx = -1;
    const int m_handle_size = 8;

    QString m_inference_result;
    QString m_keywords;
    QVector<QString> m_all_keywords;
    bool m_is_normal_status = true;


    void draw_status_indicator(QPainter& painter);
    void draw_inference_result(QPainter& painter);

    void update_hover_state(const QPoint& pos);
    QRect paint_close_btn(const QRect& rect) const;
    QRect get_expand_btn_rect() const;
    void handle_rect_number_changed(int idx);

    void check_resize_handles(const QPoint& pos);
    QRect get_resize_handle_rect(const QRect& rect, resize_handle handle) const;
    void update_rect_with_resize(const QPoint& pos);
    void draw_resize_handles(QPainter& painter, const QRect& rect);
};

#endif
