
#include <components/draw_overlay.h>


draw_overlay::draw_overlay(QWidget* parent)
    : QWidget{parent}
    , m_number_combobox{new QComboBox()}
    {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_TranslucentBackground);

    m_number_combobox->addItems({"1", "2", "3"});
    m_number_combobox->hide();
    m_number_combobox->setWindowFlags(Qt::Popup);
    connect ( m_number_combobox
            , QOverload<int>::of(&QComboBox::activated)
            , this
            , &draw_overlay::handle_rect_number_changed
            ) ;
}

QVector<QRect> draw_overlay::selections() const {  }

void draw_overlay::handle_rect_number_changed(int index) {
    if (m_edit_index >= 0 && m_edit_index < m_rects.size()) {
        m_rects[m_drag_idx].number = index + 1;
        update();
        qDebug() << "idx changed: " << m_rects[m_drag_idx].number;
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
            m_number_combobox->move(pos);
            m_number_combobox->show();
            m_number_combobox->setCurrentIndex(m_rects[m_hover_idx].number - 1);
        }
    } else if (e->button() == Qt::LeftButton) {
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

            qDebug() << "rd_number: " << rd.number;
            painter.drawText(rd.rect.adjusted(5, 5, 0, 0), Qt::AlignLeft | Qt::AlignTop, QString::number(rd.number));
        
    }
    if (m_is_dragging) {
        painter.setPen(QPen(Qt::blue, 2));
        painter.setBrush(QColor(0, 0, 255, 30));
        painter.drawRect(m_current_rect);
    }
}

bool draw_overlay::event(QEvent* e) {
    if (e->type() == QEvent::MouseButtonPress && m_number_combobox->isVisible()) {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        if (!m_number_combobox->geometry().contains(me->globalPos())) {
            m_number_combobox->hide();
        }
    }
    return QWidget::event(e);
}
