#include "shortcutbar.h"
#include "ui_shortcutbar.h"

ShortcutBar::ShortcutBar(QWidget *parent) :
    SideBarBase(parent),
    ui(new Ui::ShortcutBar)
{
    ui->setupUi(this);
    setObjectName("ShortcutBar");
    setupBar(tr("Shortcuts"), false);

    ui->twShortcuts->header()->setResizeMode(0, QHeaderView::Fixed);
    ui->twShortcuts->header()->resizeSection(0, 40);
    ui->twShortcuts->header()->setResizeMode(1, QHeaderView::Stretch);
}

ShortcutBar::~ShortcutBar()
{
    delete ui;
}

void ShortcutBar::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ShortcutBar::addItem(QAction *act)
{
  if (act->menu())
    return;

  if (act->isSeparator())
  {
//    QTreeWidgetItem *wi = new QTreeWidgetItem(ui->twShortcuts);
//    wi->setBackgroundColor(0, 0xeeeeee);
//    wi->setBackgroundColor(1, 0xeeeeee);
//    wi->setBackgroundColor(2, 0xeeeeee);
    return;
  }

  QString text(act->statusTip());
  if (text.isEmpty()) text = act->text();
  if (text.isEmpty()) text = act->toolTip();
  if (text.isEmpty())
    return;

  QTreeWidgetItem *wi = new QTreeWidgetItem(ui->twShortcuts);
  wi->setIcon(0, act->icon());
  wi->setText(1, text);
  wi->setText(2, act->shortcut().toString());
}

void ShortcutBar::addItems(QList<QAction*> &items)
{
  foreach(QAction *act, items)
  {
    addItem(act);
  }
}

void ShortcutBar::addItems(QMenu *menu)
{
  QTreeWidgetItem *wi = new QTreeWidgetItem(ui->twShortcuts);
  wi->setBackgroundColor(0, Qt::gray);
  wi->setBackgroundColor(1, Qt::gray);
  wi->setBackgroundColor(2, Qt::gray);
  wi->setIcon(0, menu->icon());
  wi->setText(1, menu->title().remove("&"));

  QList<QAction*> items = menu->actions();
  addItems(items);
}
