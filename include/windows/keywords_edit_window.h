

#ifndef KEYWORDS_EDIT_WINDOW_H
#define KEYWORDS_EDIT_WINDOW_H

#include <QWidget>
#include <qboxlayout.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <QLabel>
#include <QSettings>


class keywords_edit_window: public QWidget {
    Q_OBJECT
public:
    explicit keywords_edit_window(QWidget* parent = nullptr);
    ~keywords_edit_window();
    
    QVector<QString> get_keywords() const;

    void set_keywords(const QVector<QString>& keywords);
signals:
    void keywords_change(const QVector<QString>& keyword_set);
private slots:
    void on_add_button_clicked();
    void on_remove_button_clicked();
    void on_confirm_button_clicked();
    void on_cancel_button_clicked();
    void on_keyword_text_changed(const QString& text);
private:

    QVBoxLayout* m_main_layout;
    QGroupBox*  m_keywords_group;
    QVBoxLayout* m_keywords_layout;

    QHBoxLayout* m_input_layout;
    QLabel* m_keyword_label;
    QLineEdit* m_keyword_input;
    QPushButton* m_add_button;

    QListWidget* m_keywords_list;

    QHBoxLayout* m_button_layout;
    QPushButton* m_remove_button;
    QPushButton* m_confirm_button;
    QPushButton* m_cancel_button;

    QLabel* m_status_label;

    QVector<QString> m_keywords;
    QSettings* m_settings;

    void setup_UI();
    void setup_connections();
    void load_keywords();
    void save_keywords();
};
#endif
