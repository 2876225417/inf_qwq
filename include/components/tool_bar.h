
#pragma once
#include <QToolBar>
#include <QMenu>
#include <components/tool_bar.h>

class tool_bar: public QToolBar{ 
    Q_OBJECT
public:
    explicit tool_bar(QWidget* parent = nullptr);
private:


};