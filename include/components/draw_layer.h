// // camera_wrapper.h
// #include <QWidget>
// #include <QLabel>
// #include <QHBoxLayout>
// #include <utils/video_capturer.h>

// class DrawLayer : public QWidget {
//     Q_OBJECT
// public:
//     explicit DrawLayer(QWidget *parent = nullptr);
//     void setRect(const QRect &rect);

// protected:
//     void paintEvent(QPaintEvent *event) override;
//     bool eventFilter(QObject *obj, QEvent *event) override;

// private:
//     QRect m_rect;
// };

// class camera_wrapper : public QWidget {
//     Q_OBJECT
// public:
//     explicit camera_wrapper(QWidget* parent = nullptr);
//     ~camera_wrapper();
    
//     void setDetectionRect(const QRect &rect); // 新增矩形设置接口

//     QHBoxLayout* camera_layout;
//     QLabel* m_video_stream;

// private slots:
//     void updateFrame(QPixmap frame);

// private:
//     video_capturer* m_video_capturer;
//     DrawLayer* m_drawLayer;
// };

// // camera_wrapper.cpp
// DrawLayer::DrawLayer(QWidget *parent) 
//     : QWidget(parent), m_rect(0, 0, 0, 0) {
//     setAttribute(Qt::WA_TransparentForMouseEvents);
//     setAttribute(Qt::WA_TranslucentBackground);
//     if (parent) {
//         parent->installEventFilter(this);
//         setGeometry(parent->rect());
//     }
// }

// bool DrawLayer::eventFilter(QObject *obj, QEvent *event) {
//     if (obj == parent() && event->type() == QEvent::Resize) {
//         setGeometry(parentWidget()->rect());
//     }
//     return QWidget::eventFilter(obj, event);
// }

// void DrawLayer::setRect(const QRect &rect) {
//     m_rect = rect;
//     update();
// }

// void DrawLayer::paintEvent(QPaintEvent *event) {
//     QPainter painter(this);
//     painter.setPen(QPen(Qt::red, 2));
//     painter.drawRect(m_rect);
//     QWidget::paintEvent(event);
// }

// camera_wrapper::camera_wrapper(QWidget* parent)
//     : QWidget(parent),
//       camera_layout(new QHBoxLayout(this)),
//       m_video_stream(new QLabel("等待视频流...", this)),
//       m_video_capturer(new video_capturer(this)),
//       m_drawLayer(new DrawLayer(m_video_stream)) 
// {
//     m_video_stream->setAlignment(Qt::AlignCenter);
//     m_video_stream->setMinimumSize(640, 480);
//     m_video_stream->setScaledContents(true);
    
//     m_drawLayer->lower(); // 确保在最上层显示
    
//     connect(m_video_capturer, &video_capturer::frameCaptured,
//             this, &camera_wrapper::updateFrame);
    
//     camera_layout->addWidget(m_video_stream);
//     setLayout(camera_layout);
// }

// void camera_wrapper::updateFrame(QPixmap frame) {
//     m_video_stream->setPixmap(frame);
// }

// void camera_wrapper::setDetectionRect(const QRect &rect) {
//     m_drawLayer->setRect(rect);
// }

// camera_wrapper::~camera_wrapper() {
//     m_video_capturer->stop();
//     delete m_video_capturer;
// }
