
#ifndef CROPPED_WRAPPER_H
#define CROPPED_WRAPPER_H

#include <components/cropped_img_wrapper.h>
#include <QGridLayout>
#include <QVector>
#include <QScrollArea>

class cropped_imgs_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit cropped_imgs_wrapper(QWidget* parent = nullptr)
        : QWidget{parent} { }
    virtual void set_image(int, QImage&) = 0;
    virtual void set_result(int, QString&) = 0;
    virtual ~cropped_imgs_wrapper() = default;
};


template<int>
class cropped_wrapper: public cropped_imgs_wrapper {
public: 
    explicit cropped_wrapper(QWidget* parent = nullptr);
    void set_image(int, QImage&) override;
    void set_result(int, QString&) override;
private:
    QVector<cropped_img_wrapper*> m_croppeds;
    void create_croppeds_wrapper();
    QGridLayout* m_croppeds_wrapper_not_gtr_4;
    QScrollArea* m_croppeds_wrapper_gtr_4;
};

extern template class cropped_wrapper<2>;
extern template class cropped_wrapper<3>;
extern template class cropped_wrapper<4>;
extern template class cropped_wrapper<5>;
extern template class cropped_wrapper<6>;

#endif