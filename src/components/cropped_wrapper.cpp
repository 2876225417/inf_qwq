
#include "components/cropped_img_wrapper.h"
#include <components/cropped_wrapper.h>
#include <qabstractspinbox.h>
#include <qboxlayout.h>
#include <qgroupbox.h>
#include <qnamespace.h>



template <int cropped_count>
cropped_wrapper<cropped_count>::cropped_wrapper(QWidget* parent)
    : cropped_imgs_wrapper{parent}
    , m_croppeds_wrapper_not_gtr_4{new QGridLayout()}
    , m_croppeds_wrapper_gtr_4{new QScrollArea()}
    {
        if constexpr (cropped_count == 1) {
            // cropped_count == 1
            m_literals_inf_wrapper = new QVBoxLayout();
            this->setLayout(m_literals_inf_wrapper);
            // image
            m_image_layout = new QGroupBox();
            m_image_wrapper = new QHBoxLayout();
            m_image = new QLabel();
            m_image->setMaximumSize(100, 100);
            m_image_wrapper->addWidget(m_image);
            m_image_layout->setLayout(m_image_wrapper);
            m_image->setStyleSheet("background: #333");
            // keywords
            m_keywords_layout = new QGroupBox();
            m_keywords_wrapper = new QHBoxLayout();
            m_keywords_label = new QLabel("关键字： ");
            m_keywords_wrapper->addWidget(m_keywords_label);
            m_keywords_layout->setLayout(m_keywords_wrapper);
            // inf result
            m_inf_result_layout = new QGroupBox("识别结果：");
            m_inf_result_wrapper = new QHBoxLayout();
            m_inf_result = new QTextBrowser();
            m_inf_result_wrapper->addWidget(m_inf_result);
            m_inf_result_layout->setLayout(m_inf_result_wrapper);

            m_literals_inf_wrapper->addWidget(m_image_layout, 5);
            m_literals_inf_wrapper->addWidget(m_keywords_layout, 1);
            m_literals_inf_wrapper->addWidget(m_inf_result_layout, 4);

            m_literals_inf_wrapper->setContentsMargins(0, 0, 0, 0);
        } else {
            this->setLayout(m_croppeds_wrapper_not_gtr_4);
            m_croppeds_wrapper_not_gtr_4->setContentsMargins(0, 0, 0, 0);
            create_croppeds_wrapper();
        }
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

    // if constexpr (cropped_count == 1) {
    //     // m_croppeds[0]->set_cropped_image_size(640, 600);
    //     // m_croppeds[0]->remove_inferred_result();
    //     // m_croppeds[0]->setFixedSize(500, 500);
    //     m_croppeds[0]->set_inffered_result(QString("None"));
    // }
    
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
    return "";
}

template <int cropped_count>
void cropped_wrapper<cropped_count>::set_result(int index, QString& res) {
    if (index >= 0 && index < m_croppeds.size()) {
        m_croppeds[index]->set_inffered_result(res);
    }
}






template class cropped_wrapper<1>;
template class cropped_wrapper<2>;
template class cropped_wrapper<3>;
template class cropped_wrapper<4>;
template class cropped_wrapper<5>;
template class cropped_wrapper<6>;
