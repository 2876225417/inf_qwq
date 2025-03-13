

#ifndef WARNING_RECORDS_WINDOW_H
#define WARNING_RECORDS_WINDOW_H

#include "opencv2/highgui.hpp"
#include <QWidget>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qendian.h>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qtablewidget.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <QLabel>
#include <QTableWidget>
#include <QDateEdit>
#include <QStyle>
#include <QDialog>
#include <QTextEdit>
#include <QTextStream>

#include <QFile>
#include <QMessageBox>
#include <QFileDialog>


class warning_records_window: public QWidget {
    Q_OBJECT
public:
    explicit warning_records_window(QWidget* parent = nullptr);
    ~warning_records_window();

    void refresh_data();

private slots:
    void on_filter_changed();

    void on_previous_page();
    void on_next_page();
    void on_page_size_changed(int size);

    
    void on_export_data();
    void on_view_details();

    void on_date_range_changed();


private:
    QVBoxLayout* m_main_layout;
    
    QGroupBox* m_filter_group;
    QGridLayout* m_filter_layout;

    QCheckBox* m_only_abnormal_check;
    QLabel* m_keyword_label;
    QLineEdit* m_keyword_edit;

    QLabel* m_date_range_label;
    QDateEdit* m_start_time_edit;
    QDateEdit* m_end_time_edit;

    QLabel* m_camera_label;
    QComboBox* m_camera_combo;

    QPushButton* m_filter_button;
    QPushButton* m_reset_button;

    QTableWidget* m_records_table;

    QGroupBox* m_pagination_group;
    QHBoxLayout* m_pagination_layout;

    QLabel* m_page_info_label;
    QPushButton* m_prev_page_button;
    QPushButton* m_next_page_button;

    QLabel* m_page_size_label;
    QComboBox* m_page_size_combo;

    QHBoxLayout* m_actions_layout;
    QPushButton* m_export_button;
    QPushButton* m_view_details_button;

    QLabel* m_status_label;

    int m_current_page = 1;
    int m_page_size = 20;
    int m_total_records = 0;
    
    bool m_only_abnormal = false;
    QString m_keyword_filter;
    QDateTime m_start_date;
    QDateTime m_end_date;
    int m_selected_camera_id = -1;

    QVector<QMap<QString, QVariant>> m_current_records;

    void setup_UI();
    void setup_connections();
    void load_data();
    void update_table();
    void update_page_info();
    void load_cameras();
    void setup_table_headers();
    QString format_date_time(const QDateTime& date_time);
    void show_status_message(const QString& message, bool is_error = false);
    void export2CSV(const QString& filename);
    void show_details_dialog(int record_index);
};
#endif
