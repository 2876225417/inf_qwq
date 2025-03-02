
#include "components/cropped_img_wrapper.h"
#include <components/cropped_wrapper.h>

template <int cropped_count>
cropped_wrapper<cropped_count>::cropped_wrapper(QWidget* parent)
    : cropped_imgs_wrapper{parent}
    , m_croppeds_wrapper_not_gtr_4{new QGridLayout()}
    , m_croppeds_wrapper_gtr_4{new QScrollArea()}
    {
    this->setLayout(m_croppeds_wrapper_not_gtr_4);
    m_croppeds_wrapper_not_gtr_4->setContentsMargins(0, 0, 0, 0);
    create_croppeds_wrapper();
}

template<int cropped_count>
void cropped_wrapper<cropped_count>::create_croppeds_wrapper() {
    const bool need_scroll = (cropped_count > 4);
    const int cols = need_scroll ? 3 : 2;
    
    QWidget* container = new QWidget();
    QGridLayout* grid_layout = new QGridLayout(container);
    grid_layout->setSpacing(10);
    grid_layout->setContentsMargins(5, 5, 5, 5);

    for (int i = 0; i < cropped_count; ++i) {
        auto cropped_part = new cropped_img_wrapper{container};
        cropped_part->setFixedSize(320, 300);
        m_croppeds.push_back(cropped_part);
        grid_layout->addWidget(cropped_part, i / cols, i % cols);
    }


    if (need_scroll) {
        m_croppeds_wrapper_gtr_4 = new QScrollArea(this);
        m_croppeds_wrapper_gtr_4->setWidgetResizable(true);
        m_croppeds_wrapper_gtr_4->setWidget(container);
        m_croppeds_wrapper_not_gtr_4->addWidget(m_croppeds_wrapper_gtr_4);

        m_croppeds_wrapper_gtr_4->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_croppeds_wrapper_gtr_4->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        m_croppeds_wrapper_not_gtr_4->addWidget(container);
    }
}

template <int cropped_count>
void cropped_wrapper<cropped_count>::set_image(int index, QImage& img) {
   if (index >= 0 && index < m_croppeds.size()) {
        m_croppeds[index]->set_cropped_image(img); 
   } 
}

template <int cropped_count>
QString cropped_wrapper<cropped_count>::get_result(int index, QImage& image) {
    if (index >= 0 && index < m_croppeds.size()) {


    }
}



template <int cropped_count>
void cropped_wrapper<cropped_count>::set_result(int index, QString& res) {
    if (index >= 0 && index < m_croppeds.size()) {
        m_croppeds[index]->set_inffered_result(res);
    }
}

template class cropped_wrapper<2>;
template class cropped_wrapper<3>;
template class cropped_wrapper<4>;
template class cropped_wrapper<5>;
template class cropped_wrapper<6>;
