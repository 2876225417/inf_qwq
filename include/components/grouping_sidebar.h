
#ifndef GROUPING_SIDEBAR_H
#define GROUPING_SIDEBAR_H

#include <qtmetamacros.h>
#include <qtoolbar.h>
class grouping_sidebar: public QToolBar{
    Q_OBJECT
public:
    explicit grouping_sidebar(int group_num, QWidget* parent = nullptr);

signals:
    void group_selected(int group_index);

private:
};




#endif // GROUPING_SIDEBAR
