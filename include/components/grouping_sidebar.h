
#ifndef GROUPING_SIDEBAR_H
#define GROUPING_SIDEBAR_H

#include <qlist.h>
#include <qtmetamacros.h>
#include <qtoolbar.h>
class grouping_sidebar: public QToolBar{
    Q_OBJECT
public:
    explicit grouping_sidebar(int group_num, QWidget* parent = nullptr);

    void set_checked(int index);
    void set_current_group(int group_index);
signals:
    void group_selected(int group_index);
private:
    QToolBar* m_toolbar;
    QList<QAction*> m_group_actions;
    int m_current_group;
};




#endif // GROUPING_SIDEBAR
