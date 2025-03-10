

#include <components/grouping_sidebar.h>
#include <qnamespace.h>
#include <qwidget.h>

grouping_sidebar::grouping_sidebar(int groups_num, QWidget* parent)
    : QToolBar{parent}
{
    
    setStyleSheet(R"(
        QToolBar {
            background: #2D2D2D;
            spacing: 1px;
            border-right: 1px solid #404040;
        }
        QToolButton {
            color: #CCCCCC;
            font: 500 13px 'Segoe UI';
            text-align: center;
            padding: 15px 6px;
            border-top: 1px solid #383838;
            border-bottom: 1px solid #222;
            min-width: 70px;
            min-height: 54px;
        }
        QToolButton:hover {
            background: #353535;
            border-left: 3px solid #4A90E2;
        }
    )");

    setOrientation(Qt::Vertical);
    setMovable(false);
    setIconSize(QSize(24,24));

    
    for (int i = 0; i < groups_num; ++i) {
        QAction* act = new QAction(
            QString("Group %1\nCh %2-%3").arg(i+1).arg(i*2+1).arg(i*2+2),
            this
        );
        act->setData(i);
        addAction(act);

        
        if(i != groups_num-1) {
            addSeparator();
            actions().last()->setSeparator(true);
        }
    }

    
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setMinimumWidth(140); 
}

