#pragma once

#include <GL/gl.h>
#include <QMainWindow>
#include <QCamera>
#include <QVideoWidget>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

#include <components/camera_wrapper.h>
#include <components/camera_config_wrapper.h>
#include <components/tool_bar.h>
#include <components/status_bar.h>
#include <components/cropped_img_wrapper.h>
#include <components/actions_wrapper.h>

#include <qevent.h>
#include <qnamespace.h>
#include <qopenglshaderprogram.h>
#include <utils/ort_inf.h>

#include <components/cropped_wrapper.h>


#include <QHBoxLayout>

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>

#include <QEvent>

class cus_button: public QOpenGLWidget, protected QOpenGLExtraFunctions {
    Q_OBJECT
public:
    explicit cus_button(QWidget* parent = nullptr)
        : QOpenGLWidget{parent}
        , QOpenGLExtraFunctions()
        , m_shader_program{new QOpenGLShaderProgram()}
        {}
    ~cus_button() {
        makeCurrent();
        glDeleteBuffers(1, &m_VBO);
        delete m_shader_program;
        doneCurrent();
    }

    void set_color(const QColor& color);

signals:
    void clicked();
protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        glClearColor(0.9f, 0.9f, 0.9f, 1.f);
        m_shader_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
            "#version 460 core\n"
            "layout(location=0) in vec2 aPos;\n"
            "uniform mat4 projection;\n"
            "void main(){\n"
            "   gl_Position = projection * vec4(aPos, 0.0, 1.0);\n"
            "}");

        m_shader_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
            "#version 460 core\n"
            "uniform vec3 color;\n"
            "out vec4 FragColor;\n"
            "void main(){\n"
            "   FragColor = vec4(color, 1.0);\n"
            "}");
        
        m_shader_program->link();
        setup_vertex_data();
    }
    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT);
        
        m_shader_program->bind();
        m_shader_program->setUniformValue("projection", m_projection);
        m_shader_program->setUniformValue("color", m_pressed ? m_base_color * 0.8f : m_base_color);

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        m_shader_program->release();
    }
    void resizeGL(int w, int h) override {
        m_projection.setToIdentity();
        m_projection.ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            m_pressed = true;
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent* e) override {
        
    }
    
private:
    QOpenGLShaderProgram* m_shader_program;
    GLuint m_VAO, m_VBO;
    QMatrix4x4 m_projection;    
    QVector3D m_base_color = { 0.2f, 0.6f, 1.f };
    bool m_pressed = false;
    
    void setup_vertex_data() {
        const float vertices[] = {
            -0.95f, -0.45f,
            0.95f, -0.45f,
        0.95f,0.45f,
        -0.95f, 0.45f
        };
        
        glGenBuffers(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};


class mainwindow: public QMainWindow {
    Q_OBJECT
public:
    explicit mainwindow(QWidget* parent = nullptr);
    ~mainwindow();
    QWidget*                m_mainwindow_layout;
    QHBoxLayout*            m_mainwindow_layout_wrapper;

    QVBoxLayout*            m_camera_rel_layout;
    QGroupBox*              m_mainwindow_camera_layout;
    QHBoxLayout*            m_mainwindow_camera_layout_wrapper;

    actions_wrapper*        m_actions_wrapper;

    QGroupBox*              m_camera_cropped_layout;
    QVBoxLayout*            m_camera_cropped_layout_wrapper;
private:

    camera_wrapper*         m_camera;
    camera_config_wrapper*  m_camera_config;

    QHBoxLayout*            m_cropped_img_1_and_2_wrapper;
    cropped_img_wrapper*    m_cropped_img_1;
    cropped_img_wrapper*    m_cropped_img_2;
    QHBoxLayout*            m_cropped_img_3_and_4_wrapper;
    cropped_img_wrapper*    m_cropped_img_3;
    cropped_img_wrapper*    m_cropped_img_4;

    cropped_wrapper<4>*     m_4_croppeds_img;

    ort_inferer*             m_ort_infer;

    cus_button* m_cus_button;

    tool_bar*               m_tool_bar;
    status_bar*             m_status_bar;
};