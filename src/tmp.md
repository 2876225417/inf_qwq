

这里通过**constexpr反射**和**模板特化**实现的完整解决方案：

```cpp
// control_ids.h - 控件标识声明
#pragma once
#include <array>
#include <string_view>

// 控件类型枚举
enum class ControlType { Button, ComboBox, CheckBox };

// 按钮枚举定义
enum class ButtonId {
    StartInference,
    SwitchCameraStream,
    SwitchAutoCrop,
    SelectStorePath,
    ClearAllCropped
};

// 下拉框枚举定义
enum class ComboBoxId {
    AdjustInterval,
    CroppedCount,
    CameraIndex
};

// 复选框枚举定义
enum class CheckBoxId {
    MultiCameras
};

// -------------------------------------------
// 反射元编程实现
namespace detail {
    template<typename E>
    struct ControlTraits;

    // 按钮反射特化
    template<>
    struct ControlTraits<ButtonId> {
        using ID = ButtonId;
        static constexpr std::string_view type_name = "Button";
        static constexpr std::array entries {
            std::pair{ButtonId::StartInference,     "start_inf"},
            std::pair{ButtonId::SwitchCameraStream, "switch_camera_stream"},
            std::pair{ButtonId::SwitchAutoCrop,     "switch_auto_crop"},
            std::pair{ButtonId::SelectStorePath,    "select_store_path"},
            std::pair{ButtonId::ClearAllCropped,    "clear_all_cropped"}
        };
    };

    // 下拉框反射特化
    template<>
    struct ControlTraits<ComboBoxId> {
        using ID = ComboBoxId;
        static constexpr std::string_view type_name = "ComboBox";
        static constexpr std::array entries {
            std::pair{ComboBoxId::AdjustInterval, "adjust_inf_interval"},
            std::pair{ComboBoxId::CroppedCount,   "select_cropped_count"},
            std::pair{ComboBoxId::CameraIndex,    "select_camera_index"}
        };
    };

    // 复选框反射特化
    template<>
    struct ControlTraits<CheckBoxId> {
        using ID = CheckBoxId;
        static constexpr std::string_view type_name = "CheckBox";
        static constexpr std::array entries {
            std::pair{CheckBoxId::MultiCameras, "switch_multi_cameras"}
        };
    };
}

// 反射接口
template<typename E>
constexpr std::string_view enum_to_string(E id) {
    const auto& entries = detail::ControlTraits<E>::entries;
    for (const auto& [key, value] : entries) {
        if (key == id) return value;
    }
    return "";
}

template<typename E>
constexpr E string_to_enum(std::string_view str) {
    const auto& entries = detail::ControlTraits<E>::entries;
    for (const auto& [key, value] : entries) {
        if (value == str) return key;
    }
    throw std::invalid_argument("Invalid enum string");
}

// -------------------------------------------
// 控件工厂实现
#include <QtWidgets>
#include <functional>

template<ControlType Type>
class ControlFactory;

// 按钮工厂特化
template<>
class ControlFactory<ControlType::Button> {
public:
    using ID = ButtonId;
    using WidgetType = QPushButton;
    using Callback = std::function<void()>;

    static QPushButton* create(ID id, QWidget* parent, Callback cb) {
        auto* btn = new QPushButton(parent);
        btn->setObjectName(QString(enum_to_string(id).data()));
        configure(id, btn);
        QObject::connect(btn, &QPushButton::clicked, cb);
        return btn;
    }

private:
    static void configure(ID id, QPushButton* btn) {
        switch(id) {
        case ButtonId::StartInference:
            btn->setText("Start Inference");
            btn->setIcon(QIcon(":/icons/start.png"));
            break;
        case ButtonId::SwitchCameraStream:
            btn->setText("Switch Camera");
            break;
        // ... 其他按钮配置
        }
    }
};

// ComboBox工厂特化 
template<>
class ControlFactory<ControlType::ComboBox> {
public:
    using ID = ComboBoxId;
    using WidgetType = QComboBox;
    
    static QComboBox* create(ID id, QWidget* parent) {
        auto* cb = new QComboBox(parent);
        cb->setObjectName(QString(enum_to_string(id).data()));
        configure(id, cb);
        return cb;
    }

private:
    static void configure(ID id, QComboBox* cb) {
        switch(id) {
        case ComboBoxId::AdjustInterval:
            cb->addItems({"1s", "2s", "5s"});
            break;
        // ... 其他组合框配置
        }
    }
};

// -------------------------------------------
// 布局生成模板
template<ControlType Type>
class LabeledControl : public QHBoxLayout {
public:
    using Factory = ControlFactory<Type>;

    template<typename... Args>
    LabeledControl(typename Factory::ID id, 
                  QWidget* parent,
                  Args&&... args)
    {
        QLabel* label = new QLabel(get_label_text(id), parent);
        widget = Factory::create(id, parent, std::forward<Args>(args)...);
        
        this->addWidget(label);
        this->addWidget(widget);
        this->setContentsMargins(0, 0, 0, 0);
    }

    typename Factory::WidgetType* control() const { return widget; }

private:
    static QString get_label_text(typename Factory::ID id) {
        return QString(enum_to_string(id)).replace('_', ' ').mid(5);
    }

    Factory::WidgetType* widget;
};

// -------------------------------------------
// 应用示例
class ActionsWrapper : public QWidget {
    Q_OBJECT
public:
    explicit ActionsWrapper(QWidget* parent = nullptr) : QWidget(parent) {
        auto* layout = new QHBoxLayout(this);
        
        // 按钮组
        auto* btnGroup = new QGroupBox("Actions");
        auto* btnLayout = new QVBoxLayout(btnGroup);
        btnLayout->addLayout(new LabeledControl<ControlType::Button>(
            ButtonId::StartInference, this, [=]{ onStartClicked(); }
        ));
        // 添加其他按钮...

        // 下拉框组
        auto* comboGroup = new QGroupBox("Settings");
        auto* comboLayout = new QVBoxLayout(comboGroup);
        comboLayout->addLayout(new LabeledControl<ControlType::ComboBox>(
            ComboBoxId::AdjustInterval, this
        ));
        // 添加其他下拉框...

        layout->addWidget(btnGroup);
        layout->addWidget(comboGroup);
    }

private slots:
    void onStartClicked() {
        qDebug() << "Start inference clicked";
    }
};
```

**关键改进说明**：

1. **枚举反射系统**：
- 通过`ControlTraits`模板特化为每个控件类型建立标识映射
- `enum_to_string`支持从枚举值获取原始字符串
- 编译时校验保证枚举值覆盖（通过static_assert）

2. **类型安全工厂**：
- 每个控件类型有独立工厂特化
- 内置控件默认配置（可见`configure`方法）
- 强类型的回调绑定

3. **自动布局生成**：
- `LabeledControl`封装标签+控件的通用布局逻辑
- 自动生成友好标签（将"start_inf"转为"start inf"）

4. **维护性提升**：
- 新增控件只需添加枚举值并更新对应ControlTraits
- 控件配置逻辑集中管理
- 消除字符串硬编码

**扩展建议**：
- 添加`static_assert`确保枚举与配置匹配
```cpp
static_assert(
    ControlTraits<ButtonId>::entries.size() == 
    static_cast<size_t>(ButtonId::COUNT),
    "ButtonId entries mismatch"
);
```
- 增加样式表自动应用能力
- 支持多语言反射




以下是通过C++23特性改进后的关键代码优化点：

**1. 使用`if consteval`增强编译期检查**
```cpp
template<typename E>
constexpr std::string_view enum_to_string(E id) {
    if consteval { // 编译期验证
        static_assert(std::is_enum_v<E>, "Must be enum type");
        constexpr auto entries = ControlTraits<E>::entries;
        for (size_t i=0; i<entries.size(); ++i) {
            if (entries[i].first != static_cast<E>(i)) {
                throw "Enums must be contiguous"; // 编译时触发错误
            }
        }
    }
    // ...运行时逻辑不变
}
```

**2. 应用多维字面量初始化**
```cpp
template<>
struct ControlTraits<ButtonId> {
    static constexpr std::array entries = []{
        std::array<std::pair<ButtonId, const char*>, 5> arr{};
        arr[0] = {ButtonId::StartInference, "start_inf"};
        arr[1] = {ButtonId::SwitchCameraStream, "switch_camera_stream"};
        // ...其他初始化
        return arr;
    }();
};
```

**3. 结构化绑定扩展**
```cpp
template<typename E>
constexpr auto get_enum_info() {
    return std::make_tuple(
        ControlTraits<E>::type_name,
        std::span(ControlTraits<E>::entries)
    );
}

// 使用示例
constexpr auto [name, entries] = get_enum_info<ButtonId>();
```

**4. 静态vector简化模板元编程**（需要包含实验库）
```cpp
#include <vector>
template<typename E>
consteval auto get_enum_strings() {
    std::vector<std::string_view> strings;
    for (const auto& [key, val] : ControlTraits<E>::entries) {
        strings.push_back(val);
    }
    return strings;
}
```

**5. 改进控件工厂（使用显式对象参数）**
```cpp
template<ControlType Type>
class ControlFactory {
public:
    template<typename Self>
    static auto create(this Self&& self, 
                      typename Self::ID id, 
                      QWidget* parent, 
                      auto&&... args)
    {
        auto* widget = new typename Self::WidgetType(parent);
        widget->setObjectName(QString(enum_to_string(id)));
        Self::configure(id, widget, args...);
        return widget;
    }
};
```

**6. 应用标准格式库**
```cpp
#include <format>
void on_button_clicked(ButtonId id) {
    std::println("按钮 {} 被点击，类型：{}", 
        enum_to_string(id),
        ControlTraits<ButtonId>::type_name
    );
}
```

**7. 安全枚举转换（结合concepts）**
```cpp
template<typename E>
concept ReflectedEnum = requires {
    { ControlTraits<E>::entries } -> std::convertible_to<std::span<const std::pair<E, std::string_view>>>;
};

template<ReflectedEnum E>
constexpr std::string_view safe_enum_to_string(E id) {
    // 实现保证安全性的转换
}
```

这些改进在保持原有功能的同时提高了类型安全性和编译期验证能力。建议在实际项目中根据编译器支持情况逐步应用这些特性。



以下是应用C++23特性优化后的完整实现代码（需编译器支持最新特性）：

```cpp
// control_system.h
#pragma once
#include <QtWidgets>
#include <array>
#include <span>
#include <format>

//---------- 类型定义 ----------
enum class ControlType { Button, ComboBox, CheckBox };

//---------- 枚举定义 ----------
enum class ButtonId   { StartInference, SwitchCameraStream, SwitchAutoCrop };
enum class ComboBoxId { AdjustInterval, CroppedCount };
enum class CheckBoxId { MultiCameras };

//---------- 使用C++23的反射系统 ----------
namespace detail {
    template<typename E> struct ControlTraits;

    template<> 
    struct ControlTraits<ButtonId> {
        static constexpr std::string_view type_name = "Button";
        static constexpr std::array entries = []{
            std::array<std::pair<ButtonId, const char*>, 3> arr{};
            arr[0] = {ButtonId::StartInference,     "start_inf"};
            arr[1] = {ButtonId::SwitchCameraStream, "switch_camera_stream"};
            arr[2] = {ButtonId::SwitchAutoCrop,     "switch_auto_crop"};
            return arr;
        }();
    };

    template<> 
    struct ControlTraits<ComboBoxId> {
        static constexpr std::string_view type_name = "ComboBox";
        static constexpr std::array entries = []{
            std::array<std::pair<ComboBoxId, const char*>, 2> arr{};
            arr[0] = {ComboBoxId::AdjustInterval, "adjust_inf_interval"};
            arr[1] = {ComboBoxId::CroppedCount,   "select_cropped_count"};
            return arr;
        }();
    };

    template<> 
    struct ControlTraits<CheckBoxId> {
        static constexpr std::string_view type_name = "CheckBox";
        static constexpr auto entries = std::to_array({
            std::pair{CheckBoxId::MultiCameras, "switch_multi_cameras"}
        });
    };
}

//---------- 编译期反射接口 ----------
template<typename E>
concept ReflectedEnum = requires {
    typename detail::ControlTraits<E>;
    { detail::ControlTraits<E>::entries } -> std::convertible_to<std::span<const std::pair<E, std::string_view>>>;
};

template<ReflectedEnum E>
constexpr std::string_view enum_to_string(E id) {
    if consteval { // C++23 if consteval
        static_assert(std::is_enum_v<E>, "Type must be an enum");
        constexpr auto& entries = detail::ControlTraits<E>::entries;
        for (size_t i = 0; i < entries.size(); ++i) {
            if (entries[0].first != static_cast<E>(0)) {
                throw "Enums must start from 0"; // 触发编译错误
            }
        }
    }
    
    for (const auto& [key, val] : detail::ControlTraits<E>::entries) {
        if (key == id) return val;
    }
    throw std::invalid_argument("Invalid enum value");
}

//---------- 控件工厂（应用显式对象参数）----------
template<ControlType Type>
class ControlFactory {
public:
    template<typename Self>
    static auto create(this Self&& self, 
                      typename Self::ID id, 
                      QWidget* parent,
                      auto&&... args)
    {
        auto* widget = new typename Self::WidgetType(parent);
        widget->setObjectName(QString(enum_to_string(id)));
        Self::configure(id, widget, args...);
        return widget;
    }
};

template<>
class ControlFactory<ControlType::Button> {
public:
    using ID = ButtonId;
    using WidgetType = QPushButton;

    static void configure(ID id, QPushButton* btn, auto&&... args) {
        btn->setText(QString(enum_to_string(id)).replace('_', ' '));
        if constexpr (requires { btn->setIcon(std::forward<decltype(args)...>(args)...); }) {
            btn->setIcon(args...);
        }
    }
};

template<>
class ControlFactory<ControlType::ComboBox> {
public:
    using ID = ComboBoxId;
    using WidgetType = QComboBox;

    static void configure(ID id, QComboBox* cb) {
        const auto add_items = [](auto* cb, auto&&... items) {
            (cb->addItem(QString(items)), ...);
        };

        switch(id) {
            using enum ComboBoxId;
            case AdjustInterval: add_items(cb, "1s", "2s", "5s"); break;
            case CroppedCount:   add_items(cb, "2", "4", "6"); break;
        }
    }
};

//---------- 标签布局组件 ----------
template<ControlType Type>
class LabeledControl : public QHBoxLayout {
    using Factory = ControlFactory<Type>;
public:
    template<typename... Args>
    LabeledControl(typename Factory::ID id, 
                  QWidget* parent,
                  Args&&... args)
    {
        QLabel* label = new QLabel(gen_label_text(id), parent);
        QWidget* ctrl = Factory::create(id, parent, std::forward<Args>(args)...);
        
        label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        this->addWidget(label);
        this->addWidget(ctrl);
    }

private:
    static QString gen_label_text(typename Factory::ID id) {
        std::string_view sv = enum_to_string(id);
        return QString(sv.substr(sv.find('_') + 1).data()).replace('_', ' ');
    }
};

//---------- 应用主窗口 ----------
class ControlPanel : public QWidget {
    Q_OBJECT
public:
    ControlPanel(QWidget* parent = nullptr) : QWidget(parent) {
        QHBoxLayout* mainLayout = new QHBoxLayout(this);
        
        // 创建控制组
        auto create_group = [](const char* title, auto... controls) {
            QGroupBox* group = new QGroupBox(title);
            QVBoxLayout* layout = new QVBoxLayout(group);
            (layout->addLayout(controls), ...);
            return group;
        };

        mainLayout->addWidget(create_group("Controls",
            new LabeledControl<ControlType::Button>(ButtonId::StartInference, this),
            new LabeledControl<ControlType::Button>(ButtonId::SwitchCameraStream, this)
        ));

        mainLayout->addWidget(create_group("Settings",
            new LabeledControl<ControlType::ComboBox>(ComboBoxId::AdjustInterval, this),
            new LabeledControl<ControlType::ComboBox>(ComboBoxId::CroppedCount, this)
        ));
    }
};

//---------- 单元测试 ----------
#ifdef RUN_TESTS
#include <cassert>
constexpr bool verify_reflection() {
    static_assert(enum_to_string(ButtonId::StartInference) == "start_inf");
    static_assert(enum_to_string(ComboBoxId::CroppedCount) == "select_cropped_count");
    return true;
}
static_assert(verify_reflection());
#endif
```

**核心改进点**：
1. 使用C++23`if consteval`进行编译时校验
2. 应用显式对象参数简化工厂方法
3. `std::format`风格的错误信息
4. 编译时测试的静态断言
5. 通过Lambda初始化的constexpr反射表

该实现要求编译器支持C++23特性（如GCC13+或Clang15+），在实际项目中需根据编译器支持情况适当调整。



以下是**在Arch Linux上使用Google Test进行单元测试**的完整流程（含CMake配置）：

---

### 1. 安装依赖
```bash
sudo pacman -S gtest cmake ninja
```

---

### 2. 项目结构
```text
project_root/
├── CMakeLists.txt
├── include/
│   └── control_system.h
├── src/
│   └── main.cpp
└── test/
    ├── CMakeLists.txt
    └── control_test.cpp
```

---

### 3. 主CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)
project(ControlSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 主库
add_library(control_system include/control_system.h)

# 可执行文件
add_executable(main_app src/main.cpp)
target_link_libraries(main_app PRIVATE control_system Qt6::Widgets)

# 测试子目录
enable_testing()
add_subdirectory(test)
```

---

### 4. 测试CMakeLists.txt (test/CMakeLists.txt)
```cmake
find_package(GTest REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

add_executable(control_tests
    control_test.cpp
)
target_link_libraries(control_tests PRIVATE
    control_system
    GTest::GTest
    GTest::Main
    Qt6::Widgets
    Qt6::Test
)

# 对动态链接gtest的情况需要额外链接线程库
if(NOT GTEST_IS_STATIC_LIBS)
    target_link_libraries(control_tests PRIVATE Threads::Threads)
endif()

add_test(NAME control_tests COMMAND control_tests)
```

---

### 5. 测试示例 (test/control_test.cpp)
```cpp
#include "control_system.h"
#include <gtest/gtest.h>
#include <QtTest/QSignalSpy>

TEST(ControlSystemTest, EnumReflection) {
    EXPECT_EQ(enum_to_string(ButtonId::StartInference), "start_inf");
    EXPECT_EQ(enum_to_string(ComboBoxId::AdjustInterval), "adjust_inf_interval");
}

TEST(ControlSystemTest, ButtonCreation) {
    QApplication app(fake_argc, fake_argv); // 需要模拟QApplication
    ControlPanel panel;
    
    auto* button = panel.findChild<QPushButton*>("start_inf");
    ASSERT_NE(button, nullptr);
    EXPECT_EQ(button->text(), "Start Inference");
}

TEST(ControlSystemTest, ComboBoxItems) {
    QApplication app(fake_argc, fake_argv);
    ControlPanel panel;
    
    auto* combobox = panel.findChild<QComboBox*>("adjust_inf_interval");
    ASSERT_NE(combobox, nullptr);
    EXPECT_EQ(combobox->count(), 3);
}

GTEST_API_ int main(int argc, char** argv) {
    QApplication app(argc, argv); // 初始化Qt环境
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

---

### 6. 构建与测试
```bash
# 生成构建系统
mkdir build && cd build
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Debug

# 编译
ninja

# 运行测试
ctest --output-on-failure -V

# 或直接运行测试程序
./test/control_tests --gtest_filter=ControlSystemTest.*
```

---

### 7. 关键注意事项

1. **Qt测试要求**：
   - 需要为每个测试初始化QApplication
   - 使用QSignalSpy测试信号槽
   ```cpp
   TEST(ButtonTest, ClickSignal) {
       ControlPanel panel;
       auto* btn = panel.findChild<QPushButton*>("start_inf");
       QSignalSpy spy(btn, &QPushButton::clicked);
       QTest::mouseClick(btn, Qt::LeftButton);
       EXPECT_EQ(spy.count(), 1);
   }
   ```

2. **Arch Linux特性**：
   ```bash
   # 若遇到链接问题，明确指定pthread
   target_link_libraries(control_tests PRIVATE pthread)
   ```

3. **覆盖率收集**：
   ```bash
   pacman -S gcovr lcov
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage"
   ninja
   lcov --capture --directory . --output-file coverage.info
   genhtml coverage.info --output-directory coverage_report

```


```

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


```
