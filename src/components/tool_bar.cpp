

#include <components/tool_bar.h>

tool_bar::tool_bar(QWidget* parent)
    : QToolBar{"ToolBar", parent}
    {
    QAction* file_act = new QAction("File", this);
    QMenu* file_menu = new QMenu(this);
    file_menu->addAction("new dialog");
    file_menu->addSeparator();
    file_menu->addAction("close");
    file_act->setMenu(file_menu);
    addAction(file_act);

    setMovable(false);
}