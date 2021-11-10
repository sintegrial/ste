#include "tabwidget.h"
#include "editwindow.h"

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent),
    lastTab(0)
{
  connect(this, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

  switchMenu = new QMenu(this);
  switchMenu->installEventFilter(this);

  connect(switchMenu, SIGNAL(hovered(QAction*)), this, SLOT(hoverTabMenu(QAction*)));
}

void TabWidget::keyPressEvent ( QKeyEvent * event )
{
  if (event->key() == Qt::Key_Tab && event->modifiers() & Qt::ControlModifier)
  {
    int idx = indexOf(lastTab);
    //qDebug() << idx;

    int aidx = -1;

    switchMenu->clear();
    for (int i = 0; i < count(); i++)
    {
      if (i == idx)
        continue;

      QAction *act = switchMenu->addAction(tabIcon(i), tabText(i));
      act->setData(i);
      act->setStatusTip(((EditWindow*)widget(i))->fileName());

      if (i == currentIndex())
      {
        // check if lastTab still exists
        if (idx >= 0)
        {
          QAction *act2 = switchMenu->addAction(tabIcon(idx), tabText(idx));
          act2->setData(idx);
          act2->setStatusTip(((EditWindow*)widget(idx))->fileName());
          aidx = idx;
        }

        switchMenu->setDefaultAction(act);
      }

    }

    if (count())
    {
      if (aidx < 0) {
        aidx = currentIndex() + 1;
        if (aidx >= count()) aidx = 0;
      }

      for (int i = 0; i < count(); i++)
        if (aidx == switchMenu->actions().at(i)->data().toInt())
        {
          switchMenu->setActiveAction(switchMenu->actions().at(i));
          break;
        }
    }

    switchMenu->show();
    int h = (height() - switchMenu->height())/2;
    int w = (width() - switchMenu->width())/2;
    QPoint r(rect().topLeft() + QPoint(w,h));
    switchMenu->move(mapToGlobal(r));

    return;
  }

  QTabWidget::keyPressEvent(event);
}

bool TabWidget::eventFilter(QObject *obj, QEvent *event)
{
  if (switchMenu->isVisible() && event->type() == QEvent::KeyRelease)
  {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

      if ((keyEvent->modifiers() & Qt::ControlModifier) == false)
      {
        if (switchMenu->isVisible())
        {
          int idx = switchMenu->activeAction()->data().toInt();
          if (idx != currentIndex())
          {
            lastTab = currentWidget();
            setCurrentIndex(idx);
          }
          switchMenu->hide();
        }
      }

      return true;

  } else {
      // standard event processing
      return QObject::eventFilter(obj, event);
  }
}

void TabWidget::tabChanged(int)
{
}

void TabWidget::hoverTabMenu(QAction* act)
{
  act->showStatusText(this);
}
