#include "mainwindow.h"
#include "editwindow.h"

#include <QtGui>
#include <Qsci/qsciprinter.h>

void MainWindow::print()
{
  printDialog->setOption(QAbstractPrintDialog::PrintSelection, textEdit->hasSelectedText());
  printDialog->setOption(QAbstractPrintDialog::PrintPageRange, false);

  int res = printDialog->exec();

  // update the tab - a workaround
  textEdit->repaint();
  QApplication::processEvents();

  if (res == QDialog::Rejected)
    return;

  if (printDialog->printRange() == QAbstractPrintDialog::Selection)
  {
    int start, end, from, to;
    textEdit->getSelection(&start, &from, &end, &to);

    printer->printRange(textEdit, start, end);
    return;
  }

  // all
  printer->printRange(textEdit);
}

