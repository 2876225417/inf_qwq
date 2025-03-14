


#include "windows/keywords_edit_window.h"
#include <qapplication.h>
#include <qboxlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qwidget.h>
#include <windows/keywords_edit_window.h>

#include <QStyle>


keywords_edit_window::keywords_edit_window(QWidget* parent)
    : QWidget{parent, Qt::Window}
    {
    setWindowTitle("关键词编辑器");
    

    m_settings = new QSettings("Chun Hui", "inf_qwq", this);

    setup_UI();
    setup_connections();
    load_keywords();
}

keywords_edit_window::~keywords_edit_window() {
    save_keywords();
}

void keywords_edit_window::setup_UI() {
    m_main_layout = new QVBoxLayout(this);
    m_main_layout->setSpacing(10);
    m_main_layout->setContentsMargins(10, 10, 10, 10);

    m_keywords_group = new QGroupBox(tr("关键词编辑"), this);
    m_keywords_layout = new QVBoxLayout();

    m_input_layout = new QHBoxLayout();
    m_keyword_label = new QLabel(tr("关键词"), this);
    m_keyword_input = new QLineEdit(this);
    m_keyword_input->setPlaceholderText("请输入关键字");
    m_add_button = new QPushButton(tr("添加"), this);
    m_add_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogYesButton));

    m_input_layout->addWidget(m_keyword_label);
    m_input_layout->addWidget(m_keyword_input);
    m_input_layout->addWidget(m_add_button);

    m_keywords_list = new QListWidget(this);
    m_keywords_list->setSelectionMode(QAbstractItemView::SingleSelection);

    m_button_layout = new QHBoxLayout();
    m_remove_button = new QPushButton(tr("移除"), this);
    m_confirm_button = new QPushButton(tr("确认"), this);
    m_cancel_button = new QPushButton(tr("取消")/*, this */);

    m_remove_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogNoButton));
    m_confirm_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    m_cancel_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

    m_button_layout->addWidget(m_remove_button);
    m_button_layout->addStretch();
    m_button_layout->addWidget(m_confirm_button);
    /* Todo: Error to cancel */
    //m_button_layout->addWidget(m_cancel_button);

    m_status_label = new QLabel(this);
    m_status_label->setWordWrap(true);

    m_keywords_layout->addLayout(m_input_layout);
    m_keywords_layout->addWidget(m_keywords_list);
    m_keywords_layout->addLayout(m_button_layout);
    m_keywords_group->setLayout(m_keywords_layout);

    m_main_layout->addWidget(m_keywords_group);
    m_main_layout->addWidget(m_status_label);

    setLayout(m_main_layout);
}

void keywords_edit_window::setup_connections() {
    connect (m_keyword_input, &QLineEdit::returnPressed, this, &keywords_edit_window::on_add_button_clicked);
    connect (m_keyword_input, &QLineEdit::textChanged, this, &keywords_edit_window::on_keyword_text_changed);
    connect (m_add_button, &QPushButton::clicked, this, &keywords_edit_window::on_add_button_clicked);
    connect (m_remove_button, &QPushButton::clicked, this, &keywords_edit_window::on_remove_button_clicked);
    connect (m_confirm_button, &QPushButton::clicked, this, &keywords_edit_window::on_confirm_button_clicked);
    connect (m_cancel_button, &QPushButton::clicked, this, &keywords_edit_window::on_cancel_button_clicked);

    m_remove_button->setEnabled(false);

    connect (m_keywords_list, &QListWidget::itemSelectionChanged, [this]() {
        m_remove_button->setEnabled(!m_keywords_list->selectedItems().isEmpty());
    });
}

void keywords_edit_window::load_keywords() {
    int size = m_settings->beginReadArray("Keywords");
    m_keywords.clear();

    for (int i = 0; i < size; ++i) {
        m_settings->setArrayIndex(i);
        QString keyword = m_settings->value("keyword").toString();
        m_keywords.append(keyword);
        m_keywords_list->addItem(keyword);
    }

    m_settings->endArray();

    if (m_keywords.isEmpty()) {
        /* probably add some default keywords */
        set_keywords({});
    }
}

void keywords_edit_window::save_keywords() {
    m_settings->beginWriteArray("Keywords");
    
    for (int i = 0; i < m_keywords.size(); ++i) {
        m_settings->setArrayIndex(i);
        m_settings->setValue("keyword", m_keywords.at(i));
    }

    m_settings->endArray();
    m_settings->sync();
}

QVector<QString> keywords_edit_window::get_keywords() const {
    return m_keywords;
}

void keywords_edit_window::set_keywords(const QVector<QString>& keywords) {
    m_keywords = keywords;
    m_keywords_list->clear();

    for (const QString& keyword: m_keywords) {
        m_keywords_list->addItem(keyword);
    }

    save_keywords();
}

void keywords_edit_window::on_add_button_clicked() {
    QString keyword = m_keyword_input->text().trimmed();

    if (keyword.isEmpty()) {
        m_status_label->setText(tr("关键字不能为空"));
        m_status_label->setStyleSheet("color: red;");
        return;
    }

    for (const QString& existing_keyword: m_keywords) {
        if (existing_keyword.toLower() == keyword.toLower()) {
            m_status_label->setText(tr("关键字 '%1' 已经存在").arg(keyword));
            m_status_label->setStyleSheet("color: red;");
            return;
        }
    }

    m_keywords.append(keyword);
    m_keywords_list->addItem(keyword);

    m_keyword_input->clear();

    m_status_label->setText(tr("关键字 '%1' 已经被添加").arg(keyword));
    m_status_label->setStyleSheet("color: green;");

}

void keywords_edit_window::on_remove_button_clicked() {
    QList<QListWidgetItem*> selected_items = m_keywords_list->selectedItems();

    if (selected_items.isEmpty()) return;

    QListWidgetItem* item = selected_items.first();
    QString keyword = item->text();

    int row = m_keywords_list->row(item);
    m_keywords_list->takeItem(row);
    m_keywords.removeAt(row);

    m_status_label->setText(tr("关键字 '%1' 已经被移除").arg(keyword));
    m_status_label->setStyleSheet("color: green");
}

void keywords_edit_window::on_confirm_button_clicked() {
    emit keywords_change(m_keywords);

    save_keywords();

    m_status_label->setText(tr("关键字已更新"));
    m_status_label->setStyleSheet("color: green");

    close();
}

void keywords_edit_window::on_cancel_button_clicked() {
    load_keywords();

    close();
}

void keywords_edit_window::on_keyword_text_changed(const QString& text) {
    m_status_label->clear();
}
