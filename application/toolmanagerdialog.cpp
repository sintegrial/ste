#include "toolmanagerdialog.h"
#include "ui_toolmanagerdialog.h"

#include "toolmanager.h"

ToolManagerDialog::ToolManagerDialog(QWidget *parent, ToolManager *toolManager) :
    QDialog(parent),
    ui(new Ui::ToolManagerDialog)
{
    ui->setupUi(this);

    // fill up
    const QList<ToolInfo>& info = toolManager->toolsInfo();
    foreach (const ToolInfo &tool, info)
    {
      QTreeWidgetItem *item = new QTreeWidgetItem(
          ui->twTools,
          QStringList() << tool.name << tool.path << tool.version
          );

      item->setCheckState(0, tool.present ? Qt::Checked : Qt::Unchecked);

      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    }

    ui->twTools->header()->setResizeMode(0,QHeaderView::ResizeToContents);
    ui->twTools->header()->setResizeMode(1,QHeaderView::Stretch);
    ui->twTools->header()->setResizeMode(2,QHeaderView::Stretch);
    ui->twTools->header()->setMovable(false);

    ui->twTools->header()->moveSection(2,1);
}

ToolManagerDialog::~ToolManagerDialog()
{
    delete ui;
}

void ToolManagerDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
