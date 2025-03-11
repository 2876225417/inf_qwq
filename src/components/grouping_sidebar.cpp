

#include <components/grouping_sidebar.h>
#include <qnamespace.h>
#include <qwidget.h>


void grouping_sidebar::set_checked(int index) {

}

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
        QToolButton:checked {
            background: #353535;
            border-left: 3px solid #4A90E2;
            color: white;
        }
    )"); 

    setOrientation(Qt::Vertical);
    setMovable(false);
    setIconSize(QSize(24,24));

    
    for (int i = 0; i < groups_num; ++i) {
        QAction* act = new QAction(
            QString("Group %1\nCh %2-%3").arg(i+1).arg(i*4+1).arg(i*4+4),
            this
        );
        act->setData(i);
        act->setCheckable(true);
        
        addAction(act);
        m_group_actions.append(act);        
        if(i != groups_num-1) {
            addSeparator();
            actions().last()->setSeparator(true);
        }
    }

    if (!m_group_actions.empty()) m_group_actions[0]->setChecked(true);

    connect ( this
            , &QToolBar::actionTriggered
            , this, [this](QAction* action) {
                int group_index = action->data().toInt();
                
                for (QAction* act: m_group_actions) {
                    act->setChecked(act == action);
                }

                if (m_current_group != group_index) {
                    m_current_group = group_index;
                    qDebug() << "Group selected: " << group_index;
                    emit group_selected(group_index);  
                }
            });
    
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setMinimumWidth(140); 
}

void grouping_sidebar::set_current_group(int group_index) {
    if (group_index < 0 || group_index >= m_group_actions.size()) return;

    if (m_current_group >= 0 && m_current_group < m_group_actions.size())
        m_group_actions[m_current_group]->setChecked(false);
    qDebug() << "Invoked change index"
             << "Group index: " << group_index;
    m_group_actions[group_index]->setChecked(true);
    m_current_group = group_index;
}


