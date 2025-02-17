#include <components/cropped_img_wrapper.h>

cropped_img_wrapper::cropped_img_wrapper(QWidget* parent)
    : QWidget{parent}
    , m_cropped_layout{new QGroupBox("Cropped", this)}
    , m_cropped_layout_wrapper{new QVBoxLayout(m_cropped_layout)}
    {

    m_cropped_image = new QLabel("Test");
    m_cropped_image->setFixedSize(400, 300);
    m_inferred_result = new QLabel("Test Label");
    m_cropped_layout_wrapper->addWidget(m_cropped_image);
    m_cropped_layout_wrapper->addWidget(m_inferred_result);
}

void cropped_img_wrapper::set_cropped_image(QImage& cropped) {
    m_cropped_image->setPixmap(QPixmap::fromImage(cropped));
}

void cropped_img_wrapper::set_inffered_result(QString& res) {
    m_inferred_result->setText(res);
}