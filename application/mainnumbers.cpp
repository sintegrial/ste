#include "mainwindow.h"
#include "numbersbar.h"
#include "editwindow.h"
#include "tabwidget.h"

void MainWindow::createNumbersBar()
{
  numbersBar = new NumbersBar(this);
  addSideBar(numbersBar);

  connect(numbersBar, SIGNAL(getNumber(QString&)), this, SLOT(getNumber(QString&)));
  connect(numbersBar, SIGNAL(pasteNumber(const QString&)), this, SLOT(pasteNumber(const QString&)));
}

void MainWindow::formatNumber()
{
  showSideBar(numbersBar);

  if (textEdit->hasSelectedText())
  {
    numbersBar->setNumber(textEdit->selectedText());
  }
}

void MainWindow::getNumber(QString &number)
{
  number = textEdit->selectedText();
}

void MainWindow::pasteNumber(const QString &number)
{
  textEdit->setFocus(Qt::MouseFocusReason);

  if (textEdit->hasSelectedText())
    textEdit->replaceSelection(number);
  else
    textEdit->insert(number);
}
