
#ifndef EXPANDED_CAMERA_WINDOW_H
#define EXPANDED_CAMERA_WINDOW_H

#include "components/cropped_wrapper.h"
#include "windows/rtsp_config_window.h"
#include <QWidget>
#include <QVBoxLayout>
#include <components/camera_wrapper.h>
#include <QPushButton>
#include <qboxlayout.h>
#include <QSplitter>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qformlayout.h>
#include <qgroupbox.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qregularexpression.h>
#include <qspinbox.h>
#include <qsyntaxhighlighter.h>
#include <qtextbrowser.h>
#include <qtextdocument.h>
#include <qtextformat.h>
#include <qwidget.h>

#include <QTextEdit>


#include <QSyntaxHighlighter>
#include <algorithm>
#include <QTextBrowser>


class keywords_highlighter: public QSyntaxHighlighter {
public:
    keywords_highlighter(QTextDocument* parent)
        : QSyntaxHighlighter { parent } { }

private:
    struct highlight_rule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<highlight_rule> rules;
};


class expanded_camera_window: public QWidget {
    Q_OBJECT
public:
    explicit expanded_camera_window( camera_wrapper* cam
                                   , QWidget* parent = nullptr);
    ~expanded_camera_window() = default;

    void set_cropped_image(const QImage& image);
    void set_rstp_info(const rtsp_config& rstp_cfg);
    
    void inline set_inf_result(const QString& inf_res) { 
        m_inf_result->setText(highlight_text(inf_res));
    }

    void inline 
    set_keywords(const QString& keywords) {
    
        m_keywords = keywords.split(QRegularExpression("[\\s\u3000]+"), Qt::SkipEmptyParts);
        m_keywords.removeDuplicates();

        m_keywords_label->setText(format_keywords_display());
        update_highlight();
    }
    QString parse_rtsp_protocal_type();

signals: 
    void keywords_changed(const QString& keywords); 
private:
    QVBoxLayout* m_expanded_camera_window_layout;
    QHBoxLayout* m_content_layout;
    QSplitter*   m_content_splitter;

    // left -> camera area
    QWidget* m_left_widget;
    QVBoxLayout* m_left_layout;
    camera_wrapper* m_camera;
    QLabel* m_title_label;

    // right -> exhibition area
    QWidget* m_right_widget;
    QVBoxLayout* m_right_layout;
    QSplitter* m_right_splitter;

    // right -> top 
    QWidget* m_right_top_widget;
    QVBoxLayout* m_right_top_layout;
    QSplitter* m_right_top_splitter;

    // right -> top -> right -> rec image
    QWidget* m_image_widget;
    QVBoxLayout* m_image_layout;
    QLabel* m_image_label;
    QLabel* m_image_display;

    // right -> top -> left  -> rec text
    QWidget* m_text_widget;
    QVBoxLayout* m_text_layout;
    QLabel* m_text_label;
    QTextEdit* m_text_display;

    // right -> bottom
    QWidget* m_settings_group;
    QVBoxLayout* m_settings_layout;
    QHBoxLayout* m_rtsp__inf_layout;
    QFormLayout* m_rtsp_info_layout;
    QFormLayout* m_inf_config_layout;
    QVBoxLayout* m_edit_layout;
    
    // right -> bottom -> rtsp connect info
    QLabel* m_rtsp_proto_type;
    QLabel* m_rtsp_username;
    QLabel* m_rtsp_ip;
    QLabel* m_rtsp_port;
    QLabel* m_rtsp_channel;
    QLabel* m_rtsp_subtype;

    // keywords edit
    QHBoxLayout* m_edit_keywords_wrapper;
    QLabel*      m_edit_keywords_label;
    QLineEdit*   m_edit_keywords;
    
    // keywords show
    QGroupBox*   m_keywords_layout;
    QHBoxLayout* m_keywords_wrapper;
    QStringList  m_keywords;
    QLabel*      m_keywords_label;
    
    // inf result
    QTextBrowser* m_inf_result;

    // comfirm 
    QPushButton* m_comfirm_config_btn;

    camera_wrapper* m_org_camera;
    rtsp_config m_rtsp_config;
    


    void setup_UI();
    void create_left_panel();
    void create_right_panel();
    void create_right_top_panel();
    void create_right_bottom_panel();
    void connect_signals();
    
    
    QString format_keywords_display() {
        if (m_keywords.isEmpty()) return "No keywords";
        return m_keywords.join("<font color='#FF5252'> • </font>");
    }

    void update_highlight() {
        if (m_inf_result && !m_inf_result->toPlainText().isEmpty()) {
            m_inf_result->setHtml(highlight_text(m_inf_result->toPlainText()));
        }
    }

    /* keywords regualrize with pattern */
    QString highlight_text(const QString& origin) const {
        if (m_keywords.isEmpty()) return origin;

        QString highlighted = origin.toHtmlEscaped();
        highlighted.replace("\n", "<br>");
        
        const QString highlight_style = 
            "style='display:inline-block; background:#FFEB3B; color: #F44336; padding:2px;'";
        
        QStringList sorted_keys = m_keywords;
        
        std::sort( sorted_keys.begin(), sorted_keys.end()
                 , [](const QString& a, const QString& b){
                    return a.length() > b.length(); 
                 });
        
        foreach (const QString& keyword, sorted_keys) {
            QString escaped_key = QRegularExpression::escape(keyword.toHtmlEscaped());
            QRegularExpression re( "(" + escaped_key + ")"
                                 , QRegularExpression::CaseInsensitiveOption 
                                 | QRegularExpression::UseUnicodePropertiesOption);
            highlighted.replace(re, QString("<span %1>\\1</span>").arg(highlight_style));
        }

        return highlighted;
    }


};

#endif  // EXPANDED_CAMERA_WINDOW_H
