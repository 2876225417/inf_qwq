
#ifndef CROPPED_WRAPPER_H
#define CROPPED_WRAPPER_H

#include <components/cropped_img_wrapper.h>
#include <QGridLayout>
#include <QVector>
#include <QScrollArea>
#include <qboxlayout.h>
#include <qforeach.h>
#include <qgroupbox.h>
#include <qnamespace.h>
#include <qregularexpression.h>
#include <qstringliteral.h>
#include <qtextdocument.h>
#include <qtextformat.h>

class cropped_imgs_wrapper: public QWidget {
    Q_OBJECT
public:
    explicit cropped_imgs_wrapper(QWidget* parent = nullptr)
        : QWidget{parent} { }
    virtual void set_image(int, QImage&) = 0;
    virtual void set_result(int, QString&) = 0;
    virtual QString get_result(int, QImage&) = 0;
    virtual ~cropped_imgs_wrapper() = default;
};

#include <QSyntaxHighlighter>

class keyword_highlighter: public QSyntaxHighlighter {
public:
    keyword_highlighter(QTextDocument* parent)
        : QSyntaxHighlighter { parent }
        {
        
    }

private:
    struct highlight_rule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<highlight_rule> rules;
};

#include <algorithm>
#include <QTextBrowser>

template<int cropped_count>
class cropped_wrapper: public cropped_imgs_wrapper {
public:
    explicit cropped_wrapper(QWidget* parent = nullptr);
    void set_image(int, QImage&) override;
    void set_result(int, QString&) override;
    QString get_result(int, QImage&) override;
    
    inline int
    get_cropped_count() const { return cropped_count; }
    inline void
    set_image(const QImage& image) { m_image->setPixmap(QPixmap::fromImage(image)); }
    inline void
    set_inf_result(const QString& inf_result) { 
        m_inf_result->setText(highlight_text(inf_result)); 
    }
    
    inline void
    set_keywords(const QString& keywords) { 
        m_keywords = keywords.split(QRegularExpression("[\\s\u3000]+"), Qt::SkipEmptyParts);
        m_keywords.removeDuplicates();

        //m_keywords = keywords_list.join(" ");
        m_keywords_label->setText(format_keywords_display());
        update_highlight();
        qDebug() << "Setted keywords!";
    }
private:
    QString format_keywords_display() {
        if (m_keywords.isEmpty()) return "No Keywords";

        return "Keywords: " + m_keywords.join("<font color='#FF5252'> • </font>");
    }

    void update_highlight() {
        if (m_inf_result && !m_inf_result->toPlainText().isEmpty()) {
            m_inf_result->setHtml(highlight_text(m_inf_result->toPlainText()));
        }
    }

    QString highlight_text(const QString& origin) const {
        if (m_keywords.isEmpty()) return origin;

        QString highlighted = origin.toHtmlEscaped();
        highlighted.replace("\n", "<br>");

        const QString highlight_style = 
            "style='display:inline-block; background:#FFEB3B; color:#F44336; padding:2px;'";

        QStringList sorted_keys = m_keywords;
        std::sort(sorted_keys.begin(), sorted_keys.end(),
                [](const QString& a, const QString& b) { return a.length() > b.length(); });

        foreach (const QString& keyword, sorted_keys) {
            QString escapedKey = QRegularExpression::escape(keyword.toHtmlEscaped());
            QRegularExpression re("(" + escapedKey + ")", 
                                QRegularExpression::CaseInsensitiveOption | 
                                QRegularExpression::UseUnicodePropertiesOption);
            highlighted.replace(re, QString("<span %1>\\1</span>").arg(highlight_style));
        }

        return highlighted;
    }
   

    QVector<cropped_img_wrapper*> m_croppeds;
    void create_croppeds_wrapper();
    QGridLayout* m_croppeds_wrapper_not_gtr_4;
    QScrollArea* m_croppeds_wrapper_gtr_4;
    
    
    // cropped_count == 1
    QVBoxLayout* m_literals_inf_wrapper;
    // image
    QGroupBox*   m_image_layout;
    QHBoxLayout* m_image_wrapper;
    QLabel*      m_image;
    
    // keywords
    QGroupBox*   m_keywords_layout;
    QHBoxLayout* m_keywords_wrapper;
    QStringList  m_keywords;
    QLabel*      m_keywords_label;

    // inf_result
    QGroupBox*   m_inf_result_layout;
    QHBoxLayout* m_inf_result_wrapper;
    QTextBrowser*      m_inf_result;

};




extern template class cropped_wrapper<1>;
extern template class cropped_wrapper<2>;
extern template class cropped_wrapper<3>;
extern template class cropped_wrapper<4>;
extern template class cropped_wrapper<5>;
extern template class cropped_wrapper<6>;

#endif
