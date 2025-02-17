#pragma once
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QEvent>
#include <QPainter>
#include <QMouseEvent>

class draw_overlay: public QWidget {
    Q_OBJECT
public:
    explicit draw_overlay(QWidget* parent = nullptr);

    QRect selection() const; 

signals:
    void selected(const QRect&);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;

private:
    QPoint  m_start;
    QRect   m_selected;
};





