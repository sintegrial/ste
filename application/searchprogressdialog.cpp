#include "searchprogressdialog.h"
#include "ui_searchprogressdialog.h"

SearchProgressDialog::SearchProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchProgressDialog)
{
    ui->setupUi(this);
}

SearchProgressDialog::~SearchProgressDialog()
{
    delete ui;
}

void SearchProgressDialog::changeEvent(QEvent *e)
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

void SearchProgressDialog::start(int count)
{
  grabMouse();
  grabKeyboard();

  ui->searchProgress->setRange(0, count);
  show();

  QApplication::processEvents();
}

void SearchProgressDialog::setProgress(int value)
{
  ui->searchProgress->setValue(value);
  QApplication::processEvents();
}

void SearchProgressDialog::hide()
{
  releaseKeyboard();
  releaseMouse();
  QDialog::hide();
}
