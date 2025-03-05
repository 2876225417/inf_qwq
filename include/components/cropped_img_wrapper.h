#pragma once


// single cropped
// image + inffered result

#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <qpoint.h>

class cropped_img_wrapper: public QWidget{ 
    Q_OBJECT 
public:
    explicit cropped_img_wrapper(QWidget* parent = nullptr);
    ~cropped_img_wrapper() = default;
    void set_cropped_image(QImage&);
    void set_inffered_result(const QString&);
    void set_cropped_image_size(int width, int height);
    void remove_inferred_result();
private:
    QGroupBox* m_cropped_layout;
    QVBoxLayout* m_cropped_layout_wrapper;
    QLabel* m_cropped_image;
    QLabel* m_inferred_result;
     

};
