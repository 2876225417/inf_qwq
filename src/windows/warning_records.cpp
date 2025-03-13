

#include "utils/db_manager_mini.h"
#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qfiledialog.h>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qoverload.h>
#include <qpushbutton.h>
#include <qtablewidget.h>
#include <qvariant.h>
#include <qwidget.h>
#include <windows/warning_records_window.h>
#include <QHeaderView>

warning_records_window::warning_records_window(QWidget* parent)
    : QWidget{parent, Qt::Window}
    {
    setWindowTitle("Warning Records");

    setup_UI();
    setup_connections();

    QDate current_date = QDate::currentDate();
    m_start_time_edit->setDate(current_date);
    m_end_time_edit->setDate(current_date);
    m_start_date = QDateTime(current_date, QTime(0, 0, 0));
    m_end_date = QDateTime(current_date, QTime(23, 59, 59));


    load_cameras();
    refresh_data();
}

warning_records_window::~warning_records_window() {}

void warning_records_window::setup_UI() {
    m_main_layout = new QVBoxLayout(this);
    m_main_layout->setSpacing(10);
    m_main_layout->setContentsMargins(10, 10, 10, 10);
    
    // filter group
    m_filter_group = new QGroupBox(tr("Filter Options"), this);
    m_filter_layout = new QGridLayout();

    m_only_abnormal_check = new QCheckBox(tr("Abnormal Records"), this);
    m_keyword_label = new QLabel(tr("Keywords:"), this);
    m_keyword_edit = new QLineEdit(this);
    m_keyword_edit->setPlaceholderText(tr("Enter keywords to filter"));

    m_date_range_label = new QLabel(tr("Date Range: "), this);
    m_start_time_edit = new QDateEdit(this);
    m_start_time_edit->setCalendarPopup(true);
    m_start_time_edit->setDisplayFormat("yyyy-MM-dd");
    m_end_time_edit = new QDateEdit(this);
    m_end_time_edit->setCalendarPopup(true);
    m_end_time_edit->setDisplayFormat("yyyy-MM-dd");

    m_camera_label = new QLabel(tr("Camera: "), this);
    m_camera_combo = new QComboBox(this);
    m_camera_combo->setMinimumWidth(150);

    m_filter_button = new QPushButton(tr("Apply Filter"), this);
    m_filter_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
    m_reset_button = new QPushButton(tr("Reset"), this);
    m_reset_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogResetButton));


    m_filter_layout->addWidget(m_only_abnormal_check, 0, 0, 1, 2);
    m_filter_layout->addWidget(m_keyword_label, 0, 2);
    m_filter_layout->addWidget(m_keyword_edit, 0, 3);
    m_filter_layout->addWidget(m_date_range_label, 1, 0);
    m_filter_layout->addWidget(m_start_time_edit, 1, 1);
    m_filter_layout->addWidget(new QLabel(tr("To"), this), 1, 2, Qt::AlignCenter);
    m_filter_layout->addWidget(m_end_time_edit, 1, 3);
    m_filter_layout->addWidget(m_camera_label, 2, 0);
    m_filter_layout->addWidget(m_camera_combo, 2, 1);
    m_filter_layout->addWidget(m_filter_button, 2, 2);
    m_filter_layout->addWidget(m_reset_button, 2, 3);

    m_filter_group->setLayout(m_filter_layout);

    m_records_table = new QTableWidget(this);
    m_records_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_records_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_records_table->setAlternatingRowColors(true);
    setup_table_headers();


    m_pagination_group = new QGroupBox(tr("Pagination"), this);
    m_pagination_layout = new QHBoxLayout();

    m_page_info_label = new QLabel(tr("Page 1 / 1 (0 records)"), this);
    m_prev_page_button = new QPushButton(tr("Prev"), this);
    m_prev_page_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowLeft));
    m_next_page_button = new QPushButton(tr("Next"), this);
    m_next_page_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowRight));

    m_page_size_label = new QLabel(tr("Records per page: "), this);
    m_page_size_combo = new QComboBox(this);

    m_page_size_combo->addItems({"10", "20", "50", "100"});
    m_page_size_combo->setCurrentText("20");

    m_pagination_layout->addWidget(m_page_info_label);
    m_pagination_layout->addStretch();
    m_pagination_layout->addWidget(m_prev_page_button);
    m_pagination_layout->addWidget(m_next_page_button);
    m_pagination_layout->addWidget(m_page_size_label);
    m_pagination_layout->addWidget(m_page_size_combo);

    m_pagination_group->setLayout(m_pagination_layout);
    
    m_actions_layout = new QHBoxLayout();
    m_export_button = new QPushButton(tr("Export"), this);
    m_export_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_view_details_button = new QPushButton(tr("Details"), this);
    m_view_details_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView));

    m_actions_layout->addWidget(m_export_button);
    m_actions_layout->addWidget(m_view_details_button);
    m_actions_layout->addStretch();

    m_status_label = new QLabel(this);
    m_status_label->setWordWrap(true);

    m_main_layout->addWidget(m_filter_group);
    m_main_layout->addWidget(m_records_table, 1);
    m_main_layout->addWidget(m_pagination_group);
    m_main_layout->addLayout(m_actions_layout);
    m_main_layout->addWidget(m_status_label);

    setLayout(m_main_layout);
}



void warning_records_window::setup_connections() {
    connect (m_filter_button, &QPushButton::clicked, this, &warning_records_window::on_filter_changed);
    connect (m_reset_button, &QPushButton::clicked, this, [this]() {
        m_only_abnormal_check->setChecked(false);
        m_keyword_edit->clear();
        m_camera_combo->setCurrentIndex(0);
        
        QDate current_date = QDate::currentDate();
        m_start_time_edit->setDate(current_date);
        m_end_time_edit->setDate(current_date);

        on_filter_changed();
    });

    connect (m_start_time_edit, &QDateEdit::dateChanged, this, &warning_records_window::on_date_range_changed);

    connect (m_end_time_edit, &QDateEdit::dateChanged, this, &warning_records_window::on_date_range_changed);

    connect (m_prev_page_button, &QPushButton::clicked, this, &warning_records_window::on_previous_page);
    connect (m_next_page_button, &QPushButton::clicked, this, &warning_records_window::on_next_page);

    connect ( m_page_size_combo
            , QOverload<int>::of(&QComboBox::currentIndexChanged)
            , [this](){ on_page_size_changed(m_page_size_combo->currentText().toInt()); });

    connect (m_export_button, &QPushButton::clicked, this, &warning_records_window::on_export_data);
    connect (m_view_details_button, &QPushButton::clicked, this, &warning_records_window::on_view_details);
    connect ( m_records_table
            , &QTableWidget::cellDoubleClicked
            , [this](int row, int col) { show_details_dialog(row); });
}

void warning_records_window::setup_table_headers() {
    QStringList headers = {
        tr("ID"), tr("Camera ID"), tr("Camera Name"), tr("Status"),
        tr("Keywords"), tr("RTSP Name"), tr("Timestamp")
    };

    m_records_table->setColumnCount(headers.size());
    m_records_table->setHorizontalHeaderLabels(headers);
    m_records_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_records_table->verticalHeader()->setVisible(false);
}

void warning_records_window::load_cameras() {
    m_camera_combo->clear();
    m_camera_combo->addItem(tr("All cameras"), -1);

    if (!db_manager::instance().is_connected()) {
        show_status_message(tr("Database not connected, can not load cameras list"), true);
        return;
    }

    QVector<QMap<QString, QVariant>> cameras = db_manager::instance().get_all_rtsp_configs();

    for (const auto& camera: cameras) {
        int id = camera["id"].toInt();
        QString name = camera["rtsp_name"].toString();
        if (name.isEmpty()) {
            name = QString("Camera %1").arg(id);
        }
        m_camera_combo->addItem(name, id);
    }
}

void warning_records_window::refresh_data() {
    m_current_page = 1;
    load_data();
}


void warning_records_window::load_data()
{
    if (!db_manager::instance().is_connected()) {
        show_status_message(tr("Database not connected，can not load data"), true);
        return;
    }
    
    QMap<QString, QVariant> countParams;
    QString countQuery = "SELECT COUNT(*) FROM warning_records WHERE 1=1";
    
    if (m_only_abnormal) {
        countQuery += " AND status = false";
    }
    
    if (!m_keyword_filter.isEmpty()) {
        countQuery += " AND (keywords LIKE :keyword OR inf_res LIKE :keyword)";
        countParams[":keyword"] = "%" + m_keyword_filter + "%";
    }
    
    if (m_start_date.isValid()) {
        countQuery += " AND record_time >= :start_date";
        countParams[":start_date"] = m_start_date;
    }
    
    if (m_end_date.isValid()) {
        countQuery += " AND record_time <= :end_date";
        countParams[":end_date"] = m_end_date;
    }
    
    if (m_selected_camera_id >= 0) {
        countQuery += " AND cam_id = :cam_id";
        countParams[":cam_id"] = m_selected_camera_id;
    }
    
    QSqlQuery count_result = db_manager::instance().execute_query(countQuery, countParams);
    if (count_result.next()) {
        m_total_records = count_result.value(0).toInt();
    } else {
        m_total_records = 0;
    }
    
    int totalPages = (m_total_records + m_page_size - 1) / m_page_size;
    if (m_current_page > totalPages && totalPages > 0) {
        m_current_page = totalPages;
    }
    
    int offset = (m_current_page - 1) * m_page_size;
    
    m_current_records = db_manager::instance().get_warning_records(
        m_selected_camera_id,
        m_only_abnormal,
        m_start_date,
        m_end_date,
        m_page_size,
        offset,
        m_keyword_filter
    );
    
    update_table();
    update_page_info();
}

void warning_records_window::update_table() {
    m_records_table->setRowCount(0);

    for (int i = 0; i < m_current_records.size(); ++i) {
        const auto& record = m_current_records[i];

        m_records_table->insertRow(i);

        m_records_table->setItem(i, 0, new QTableWidgetItem(record["id"].toString()));
        m_records_table->setItem(i, 1, new QTableWidgetItem(record["cam_id"].toString()));
        m_records_table->setItem(i, 2, new QTableWidgetItem(record["cam_name"].toString()));

        bool status = record["status"].toBool();
        QTableWidgetItem* status_item = new QTableWidgetItem(status ? tr("Normal") : tr("Abnormal"));
        status_item->setForeground(status ? Qt::green : Qt::red);
        status_item->setTextAlignment(Qt::AlignCenter);
        m_records_table->setItem(i, 3, status_item);

        m_records_table->setItem(i, 4, new QTableWidgetItem(record["keywords"].toString()));
        m_records_table->setItem(i, 5, new QTableWidgetItem(record["rtsp_name"].toString()));
        QDateTime record_time = record["record_date"].toDateTime();
        m_records_table->setItem(i, 6, new QTableWidgetItem(format_date_time(record_time)));
    }

    if (m_current_records.isEmpty()) show_status_message(tr("No matched records found"));
    else m_status_label->clear();
}

void warning_records_window::update_page_info() {
    int total_pages = (m_total_records + m_page_size - 1) / m_page_size;
    if (total_pages == 0) total_pages = 1;
    
    m_page_info_label->setText(tr("Page %1 / %2 (%3 Pages)")
                               .arg(m_current_page)
                               .arg(total_pages)
                               .arg(m_total_records));

    m_prev_page_button->setEnabled(m_current_page > 1);
    m_next_page_button->setEnabled(m_current_page < total_pages);
}

QString warning_records_window::format_date_time(const QDateTime& date_time) {
    return date_time.toString("yyyy-MM-dd hh:mm:ss");
}

void warning_records_window::show_status_message(const QString& message, bool is_error) {
    m_status_label->setText(message);
    m_status_label->setStyleSheet(is_error ? "color: red;" : "color:black;");
}

void warning_records_window::on_filter_changed() {
    m_only_abnormal = m_only_abnormal_check->isChecked();
    m_keyword_filter = m_keyword_edit->text().trimmed();
    m_selected_camera_id = m_camera_combo->currentData().toInt();

    refresh_data();
}

void warning_records_window::on_date_range_changed() {
    QDate start_date = m_start_time_edit->date();
    QDate end_date = m_end_time_edit->date();

    if (start_date > end_date) {
        m_end_time_edit->setDate(start_date);
        end_date = start_date;
    }

    m_start_date = QDateTime(start_date, QTime(0, 0, 0));
    m_end_date = QDateTime(end_date, QTime(23, 59, 59));
}

void warning_records_window::on_previous_page() {
    if (m_current_page > 1) {
        m_current_page--;
        load_data();
    }
}

void warning_records_window::on_next_page() {
    int total_pages = (m_total_records + m_page_size - 1) / m_page_size;
    if (m_current_page < total_pages) {
        m_current_page++;
        load_data();
    }
}

void warning_records_window::on_page_size_changed(int size) {
    m_page_size = size;
    m_current_page = 1;
    load_data();
}

void warning_records_window::on_export_data() {
    if (m_current_records.isEmpty()) {
        QMessageBox::warning(this, tr("Export Data"), tr("No Data to Export"));
        return;
    }

    QString file_name = QFileDialog::getSaveFileName(this, tr("Export Data"), QString("warning_records_%1.csv")
                                                                                .arg( QDateTime::currentDateTime()
                                                                                .toString("yyyyMMdd_hhmmss")), tr("CSVFile (*csv)"));

    if (file_name.isEmpty()) return;

    export2CSV(file_name);

}


void warning_records_window::on_view_details() {
    QModelIndexList selection = m_records_table->selectionModel()->selectedRows();

    if (selection.isEmpty()) {
        QMessageBox::warning(this, tr("Check details"), tr("Select a records"));
        return;
    }
    int row = selection.first().row();
    show_details_dialog(row);
}



void warning_records_window::export2CSV(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("导出错误"), tr("无法打开文件进行写入"));
        return;
    }
    
    QTextStream out(&file);
    // out.setCodec("UTF-8");
    
    // 写入BOM (Byte Order Mark)，确保Excel正确识别UTF-8
    out << QChar(0xFEFF);
    
    // 写入表头
    out << "ID,摄像头ID,摄像头名称,状态,关键词,RTSP名称,RTSP URL,推理内容,时间\n";
    
    // 写入数据
    for (const auto& record : m_current_records) {
        out << record["id"].toString() << ","
            << record["cam_id"].toString() << ","
            << "\"" << record["cam_name"].toString().replace("\"", "\"\"") << "\","
            << (record["status"].toBool() ? "正常" : "异常") << ","
            << "\"" << record["keywords"].toString().replace("\"", "\"\"") << "\","
            << "\"" << record["rtsp_name"].toString().replace("\"", "\"\"") << "\","
            << "\"" << record["rtsp_url"].toString().replace("\"", "\"\"") << "\","
            << "\"" << record["inf_res"].toString().replace("\"", "\"\"") << "\","
            << "\"" << format_date_time(record["record_time"].toDateTime()) << "\"\n";
    }
    
    file.close();
    
    QMessageBox::information(this, tr("导出成功"), 
                            tr("数据已成功导出到: %1").arg(filename));
}



void warning_records_window::show_details_dialog(int record_index)
{
    if (record_index < 0 || record_index >= m_current_records.size()) {
        return;
    }
    
    const auto& record = m_current_records[record_index];
    
    QDialog dialog(this);
    dialog.setWindowTitle(tr("记录详情"));
    dialog.setMinimumSize(600, 400);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QTextEdit* details_text = new QTextEdit(&dialog);
    details_text->setReadOnly(true);
    
    QString details = tr("<h2>记录详情</h2>");
    details += tr("<p><b>ID:</b> %1</p>").arg(record["id"].toString());
    details += tr("<p><b>摄像头ID:</b> %1</p>").arg(record["cam_id"].toString());
    details += tr("<p><b>摄像头名称:</b> %1</p>").arg(record["cam_name"].toString());
    details += tr("<p><b>状态:</b> <span style='color: %2;'>%1</span></p>")
              .arg(record["status"].toBool() ? tr("正常") : tr("异常"))
              .arg(record["status"].toBool() ? "green" : "red");
    details += tr("<p><b>关键词:</b> %1</p>").arg(record["keywords"].toString());
    details += tr("<p><b>RTSP名称:</b> %1</p>").arg(record["rtsp_name"].toString());
    details += tr("<p><b>RTSP URL:</b> %1</p>").arg(record["rtsp_url"].toString());
    details += tr("<p><b>时间:</b> %1</p>").arg(format_date_time(record["record_time"].toDateTime()));
    details += tr("<p><b>推理内容:</b></p>");
    details += tr("<div style='background-color: #f0f0f0; padding: 10px; border-radius: 5px;'>%1</div>")
              .arg(record["inf_res"].toString());
    
    details_text->setHtml(details);
    
    QHBoxLayout* button_layout = new QHBoxLayout();
    QPushButton* closeButton = new QPushButton(tr("关闭"), &dialog);
    button_layout->addStretch();
    button_layout->addWidget(closeButton);
    
    layout->addWidget(details_text);
    layout->addLayout(button_layout);
    
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    
    dialog.exec();
}
