#include "wordformatbar.h"
#include "ui_wordformatbar.h"

WordFormatBar::WordFormatBar(QWidget *parent) :
    SideBarBase(parent),
    ui(new Ui::WordFormatBar)
{
    ui->setupUi(this);
    setObjectName("WordFormatBar");
    setupBar(tr("Format Words"), true);
}

WordFormatBar::~WordFormatBar()
{
    delete ui;
}

void WordFormatBar::changeEvent(QEvent *e)
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

void WordFormatBar::on_pbFormat_clicked()
{
  FormatInfo info;

  if (ui->gbWords->isChecked())
    info.count = ui->sbWords->value();
  else
    info.count = 0;

  if (ui->cbFillSpaces->isChecked())
    info.fill.fill(' ', ui->sbFillBlanks->value());
  else
    info.fill = ui->leFillPattern->text();

  if (ui->cbCell->isChecked())
    info.cellWidth = ui->sbCell->value();
  else
    info.cellWidth = 0;

  info.cellExpand = (ui->cbFitWords->isChecked());

//  info.cellAlign = ui->rbAlignLeft->isChecked() ? Qt::AlignLeft :
//                   ui->rbAlignRight->isChecked() ? Qt::AlignRight :
//                   Qt::AlignCenter;

  emit format(info);
}
