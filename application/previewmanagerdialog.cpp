#include "previewmanagerdialog.h"
#include "ui_previewmanagerdialog.h"

PreviewManagerDialog::PreviewManagerDialog(QWidget *parent) :
    QDialog(parent,
            Qt::CustomizeWindowHint | Qt::WindowTitleHint |
            Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint
            ),
    ui(new Ui::PreviewManagerDialog)
{
    ui->setupUi(this);

    setMinimumSize(640, 480);

    ui->view->setLayout(new QVBoxLayout());
    ui->view->layout()->setMargin(0);
}

PreviewManagerDialog::~PreviewManagerDialog()
{
    delete ui;
}

void PreviewManagerDialog::changeEvent(QEvent *e)
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

void PreviewManagerDialog::preview(QWidget *output)
{
  //QTime tick; tick.start();

  ui->view->layout()->addWidget(output);
  output->setFocus();

  //qDebug() << "preview: " << tick.elapsed();

  exec();

  // cleanup
  ui->view->layout()->removeWidget(output);
  delete output;
}
