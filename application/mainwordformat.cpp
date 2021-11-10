#include "mainwindow.h"
#include "editwindow.h"
#include "wordformatbar.h"

////////////////////////////////////////////////////////////////////////////////

void MainWindow::createWordFormatBar()
{
  wordFormatBar = new WordFormatBar(this);
  addSideBar(wordFormatBar);
}

void MainWindow::formatWords()
{
  showSideBar(wordFormatBar);
}

void MainWindow::joinLines()
{
  bool ok;
  QString sep = QInputDialog::getText(this, tr("STE - join lines"),
                                  tr("Join selected lines using separator:"),
                                  QLineEdit::Normal, "", &ok);
  if (!ok) return;

  textEdit->joinLines(sep);
}
