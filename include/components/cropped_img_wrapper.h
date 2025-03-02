#pragma once


// single cropped
// image + inffered result

#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

class cropped_img_wrapper: public QWidget{ 
    Q_OBJECT 
public:
    explicit cropped_img_wrapper(QWidget* parent = nullptr);
    ~cropped_img_wrapper() = default;
    void set_cropped_image(QImage&);
    void set_inffered_result(QString&);
private:
    QGroupBox* m_cropped_layout;
    QVBoxLayout* m_cropped_layout_wrapper;
    QLabel* m_cropped_image;
    QLabel* m_inferred_result;
};
