#include "filesdialog.h"
#include "ui_filesdialog.h"

FilesDialog::FilesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilesDialog)
{
    ui->setupUi(this);

    ui->buttonBar->setLayout(new QHBoxLayout());
    QSpacerItem *si = new QSpacerItem(10,10, QSizePolicy::MinimumExpanding);
    ui->buttonBar->layout()->addItem(si);
    ui->treeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);

    group = new QButtonGroup(this);
    connect(group, SIGNAL(buttonClicked(int)), this, SLOT(done(int)));
}

FilesDialog::~FilesDialog()
{
    delete ui;
}

int FilesDialog::itemCount() const
{
  return ui->treeWidget->topLevelItemCount();
}

void FilesDialog::changeEvent(QEvent *e)
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

void FilesDialog::setText(const QString &header, const QString &top, const QString &bottom)
{
  setWindowTitle(header);
  ui->topLabel->setText(top);
  ui->bottomLabel->setText(bottom);
}

void FilesDialog::addButton(const QString &text, int res, bool def)
{
  QPushButton *pb = new QPushButton(text, this);
  if (def) pb->setDefault(true);
  group->addButton(pb, res);
  ui->buttonBar->layout()->addWidget(pb);
}

void FilesDialog::addItem(const QString &text)
{
  QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
  item->setText(0, text);
  ui->treeWidget->addTopLevelItem(item);
}
