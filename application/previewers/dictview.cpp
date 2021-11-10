#include "dictview.h"
#include "ui_dictview.h"

DictView::DictView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DictView)
{
    ui->setupUi(this);
    ui->gridLayout->setMargin(0);

    connect(ui->pbHome, SIGNAL(clicked()), this, SIGNAL(home()));
    connect(ui->pbEnd, SIGNAL(clicked()), this, SIGNAL(end()));
    connect(ui->pbPrev, SIGNAL(clicked()), this, SIGNAL(prev()));
    connect(ui->pbNext, SIGNAL(clicked()), this, SIGNAL(next()));
}

DictView::~DictView()
{
    delete ui;
}

void DictView::changeEvent(QEvent *e)
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

void DictView::output(DictViewInfo *info)
{
  ui->view->setHtml(info->text);
  ui->lInfo->setText(tr("Entries: %1-%2 of %3")
                     .arg(info->current+1)
                     .arg(info->current+info->shown)
                     .arg(info->entries.count()-1)
                     );

  ui->pbHome->setEnabled(info->current > 0);
  ui->pbPrev->setEnabled(ui->pbHome->isEnabled());
  ui->pbEnd->setEnabled(info->current+info->shown < info->entries.count()-1);
  ui->pbNext->setEnabled(ui->pbEnd->isEnabled());
}

void DictView::setFocus()
{
  ui->view->setFocus();
}
