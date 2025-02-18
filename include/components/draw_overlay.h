#pragma once
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QVector>
#include <QComboBox>
struct rect_data {
    QRect rect;
    bool hovered = false;
    int number = 0;
};


class draw_overlay: public QWidget {
    Q_OBJECT
public:
    explicit draw_overlay(QWidget* parent = nullptr);

    QVector<QRect> selections() const; 

signals:
    void selected(QVector<rect_data>&);
    void update_frame_moving(QVector<rect_data>&);
private slots:
    void handle_rect_number_changed(int index);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;
    bool event(QEvent*) override;
    bool eventFilter(QObject*, QEvent*) override;
private:
    QRect paint_close_btn(const QRect& rect) const;
    void update_hover_state(const QPoint& pos);

    QPoint  m_start;
    bool m_is_dragging = false;
    QVector<rect_data> m_rects;
    QRect   m_current_rect;
    int m_hover_idx = -1;
    int m_drag_idx = -1;
    QPoint m_drag_start_pos;
    QComboBox* m_number_combobox;
    int m_edit_index = -1;
};





