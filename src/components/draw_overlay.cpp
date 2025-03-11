
#include "HCNetSDK.h"
#include <components/draw_overlay.h>
#include <QTimer>

draw_overlay::draw_overlay(int cam_id, QWidget* parent)
    : QWidget{parent}
    , m_cam_id{cam_id}
    , m_number_combobox{new QComboBox(this)}
    , m_timer{new QTimer(this)}
    {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_TranslucentBackground);


    m_number_combobox->addItems({"1", "2", "3", "4"});
    m_number_combobox->hide();
    m_number_combobox->setWindowFlags(Qt::Popup);
    m_number_combobox->installEventFilter(this);
    connect ( m_number_combobox
            , QOverload<int>::of(&QComboBox::activated)
            , this
            , &draw_overlay::handle_rect_number_changed
            ) ;

    connect ( m_timer, &QTimer::timeout, this, [this]() {
               if (!m_rects.isEmpty()) emit timer_timeout_update(m_rects); 
               else qDebug() << "No rects selected.";
            } ); m_timer->start(5000);
}

QRect draw_overlay::get_expand_btn_rect() const {
    const int btn_size = 24;
    const int margin = 10;
    return QRect (
        width() - btn_size - margin,
        margin,
        btn_size,
        btn_size
    );
}


void draw_overlay::handle_rect_number_changed(int index) {
    if (m_edit_index >= 0 && m_edit_index < m_rects.size()) {
        m_rects[m_edit_index].number = index + 1;
        update();
        qDebug() << "idx changed: " << m_rects[m_drag_idx].number;
        for(const auto& rd: m_rects)
            qDebug() << "number: " << rd.number;
    }
    m_number_combobox->hide();
    m_edit_index = -1;
}

QRect draw_overlay::paint_close_btn(const QRect& rect) const {
    const int btn_size = 12;
    return QRect (
        rect.right() - btn_size - 2,
        rect.top() + 2,
        btn_size, 
        btn_size
    );
}



void draw_overlay::update_hover_state(const QPoint& pos) {
    m_expand_btn_hovered = get_expand_btn_rect().contains(pos);

    m_hover_idx = -1;
    for (int i = 0; i < m_rects.size(); ++i) {
        QRect btn_rect = paint_close_btn(m_rects[i].rect);
        if (btn_rect.contains(pos)) {
            m_hover_idx = i;
            return;
        }
        if (m_rects[i].rect.contains(pos)) {
            m_hover_idx = i;
        }
    }
}

void draw_overlay::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::RightButton){
        update_hover_state(e->pos());
        if (m_hover_idx != -1) {
            m_edit_index = m_hover_idx;
            QRect target_rect = m_rects[m_hover_idx].rect;
            QPoint pos = mapToGlobal(target_rect.topRight() + QPoint(-50, 5));
            
            QPoint center_pos = mapToGlobal(target_rect.center());
            QSize combobox_size = m_number_combobox->sizeHint();
            m_number_combobox->move(center_pos - QPoint(combobox_size.width() / 2, combobox_size.height()));
            m_number_combobox->raise();
            m_number_combobox->show();
            m_number_combobox->setCurrentIndex(m_rects[m_hover_idx].number - 1);
        }
    } else if (e->button() == Qt::LeftButton) {
        if (m_expand_btn_hovered) { emit expand_camera_request(m_cam_id); return; }

        update_hover_state(e->pos());

        if (m_hover_idx != -1) {
            QRect btn_rect = paint_close_btn(m_rects[m_hover_idx].rect);
            if (btn_rect.contains(e->pos())) {
                m_rects.remove(m_hover_idx);
                update();
                return;
            }
        }

        if (m_hover_idx != -1) {
            m_drag_idx = m_hover_idx;
            m_drag_start_pos = e->pos();
        } else {
            m_is_dragging = true;
            m_start = e->pos();
            m_current_rect = QRect();
        }
    }
    update();
}

void draw_overlay::mouseMoveEvent(QMouseEvent* e) {
    update_hover_state(e->pos());

    if (m_drag_idx != -1) {
        QPoint delta = e->pos() - m_drag_start_pos;
        m_rects[m_drag_idx].rect.translate(delta);
        m_drag_start_pos = e->pos();
        qDebug() << "Moving";
        emit update_frame_moving(m_rects);
        update();
    } else if (m_is_dragging) {
        m_current_rect = QRect(m_start, e->pos()).normalized();
        update();
    }
}

void draw_overlay::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        if (m_is_dragging && m_current_rect.isValid()) {
            rect_data new_rect;
            new_rect.rect = m_current_rect;
            m_rects.append(new_rect);
            qDebug() << "Rect added, count of rects: " 
                     << m_rects.size()
                     << "rect: " << new_rect.rect;
            emit selected(m_rects);
            m_current_rect = QRect();
        }
        m_is_dragging = false;
        m_drag_idx = -1;
    }
    update();
}

void draw_overlay::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < m_rects.size(); ++i) {
        const auto& rd = m_rects[i];
        
        painter.setPen(QPen(i == m_hover_idx ? Qt::cyan : Qt::red, 2));
        painter.setBrush(QColor(255, 0, 0, i == m_hover_idx ? 80 : 30));
        painter.drawRect(rd.rect);

        if (i == m_hover_idx) {
            QRect btn = paint_close_btn(rd.rect);
            painter.setPen(QPen(Qt::white, 2));
            painter.drawLine(btn.topLeft(), btn.bottomRight());
            painter.drawLine(btn.topRight(), btn.bottomLeft());
        }

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.drawText( rd.rect.adjusted(5, 5, 0, 0), Qt::AlignLeft 
                        | Qt::AlignTop, QString::number(rd.number) ); 
    }
    if (m_is_dragging) {
        painter.setPen(QPen(Qt::blue, 2));
        painter.setBrush(QColor(0, 0, 255, 30));
        painter.drawRect(m_current_rect);
    }
    
    QRect expand_btn = get_expand_btn_rect();
    painter.setPen(Qt::NoPen);

    if (m_expand_btn_hovered) painter.setBrush(QColor(100, 100, 255, 180));
    else painter.setBrush(QColor(70, 70, 200, 150));

    painter.drawEllipse(expand_btn);

    painter.setPen(QPen(Qt::white, 2));
    int center_X = expand_btn.center().x();
    int center_Y = expand_btn.center().y();
    int icon_size = expand_btn.width() / 3;

    painter.drawEllipse(QPoint(center_X - 2, center_Y - 2), icon_size / 2, icon_size / 2);
    
    painter.drawLine(center_X + icon_size / 3, center_Y + icon_size / 3,
                     center_X + icon_size / 2, center_Y + icon_size / 2);
}

bool draw_overlay::eventFilter(QObject* obj, QEvent* e) {
    if (obj == m_number_combobox && e->type() == QEvent::Hide) {
        m_edit_index = -1;
    }
    return QWidget::eventFilter(obj, e);
}

bool draw_overlay::event(QEvent* e) {
    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);

        if (m_number_combobox->isVisible()) {
            QPoint click_pos = me->globalPos();
            QRect combobox_geo = m_number_combobox->geometry();

            if (!combobox_geo.contains(click_pos)) {
                m_number_combobox->hide();
                update();
            }
        }
    }
    return QWidget::event(e);
}
