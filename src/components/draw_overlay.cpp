
#include <components/draw_overlay.h>


draw_overlay::draw_overlay(QWidget* parent)
    : QWidget{parent}
    {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_TranslucentBackground);
}

QRect draw_overlay::selection() const { return m_selected; }

void draw_overlay::mousePressEvent(QMouseEvent* e) {
    m_start = e->pos();
    m_selected = QRect();
    update();
}

void draw_overlay::mouseMoveEvent(QMouseEvent* e) {
    m_selected = QRect(m_start, e->pos()).normalized();
    update();
}

void draw_overlay::mouseReleaseEvent(QMouseEvent* e) {
    emit selected(m_selected);
}

void draw_overlay::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setPen(QPen(Qt::red, 2));
    painter.setBrush(QColor(255, 0, 0, 50));
    painter.drawRect(m_selected);
}

