#include "wrapdialog.h"
#include "ui_wrapdialog.h"
#include "utilsui.h"

WrapDialog::WrapDialog(QWidget *parent) :
    QDialog(parent, Qt::Popup),
    ui(new Ui::WrapDialog)
{
    ui->setupUi(this);

    connect(ui->cbStart->lineEdit(), SIGNAL(returnPressed()),
            this, SLOT(accept()));
    connect(ui->cbEnd->lineEdit(), SIGNAL(returnPressed()),
            this, SLOT(accept()));

    connect(ui->cbStart->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(text1changed(QString)));

    QCompleter *comp1 = new QCompleter(this);
    comp1->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
//    comp1->setWrapAround(false);
    comp1->setWidget(ui->cbStart);

    QCompleter *comp2 = new QCompleter(this);
    comp2->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
//    comp2->setWrapAround(false);
    comp2->setWidget(ui->cbEnd);
}

WrapDialog::~WrapDialog()
{
    delete ui;
}

void WrapDialog::changeEvent(QEvent *e)
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

int WrapDialog::exec()
{
  ui->cbStart->setFocus();
  ui->cbStart->lineEdit()->selectAll();

  QRect r = rect();
  r.moveCenter(parentWidget()->rect().center());
  move(r.topLeft());

  return QDialog::exec();
}

void WrapDialog::getText(QString &s1, QString &s2)
{
  s1 = ui->cbStart->currentText();
  s2 = ui->cbEnd->currentText();
}

void WrapDialog::text1changed(QString text)
{
  if (ui->tbNoAuto->isChecked())
    return;

  if (ui->tbTag->isChecked())
  {
    // xml tag?
    if (text.startsWith('<'))
    {
      if (text.length() == 1 || text.at(1) != '/')
        text.insert(1, '/');
    }
    else
    // dsl tag?
    if (text.startsWith('['))
    {
      text.insert(1, '/');

      if (text.length() > 0)
      if (text.at(text.length()-1).isDigit())
        text.remove(text.length()-1, 1);

      if (text.length() > 1)
      if (text.at(text.length()-2).isDigit())
        text.remove(text.length()-2, 1);
    }
  }

  ui->cbEnd->lineEdit()->setText(text);
}

void WrapDialog::readConfig(QSettings &set)
{
  UtilsUI::loadCombo(set, "wrapStartCompleter", ui->cbStart);
  UtilsUI::loadCombo(set, "wrapEndCompleter", ui->cbEnd);
}

void WrapDialog::writeConfig(QSettings &set)
{
  UtilsUI::storeCombo(set, "wrapStartCompleter", ui->cbStart);
  UtilsUI::storeCombo(set, "wrapEndCompleter", ui->cbEnd);
}
