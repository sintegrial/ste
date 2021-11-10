#include "mainwindow.h"
#include "editwindow.h"
#include "editdefines.h"

void MainWindow::createTrimBar()
{
  QMenu *trimEOLsMenu = new QMenu(this);
  connect(trimEOLsMenu, SIGNAL(triggered(QAction*)), this, SLOT(trimEolMenu_activated(QAction*)));

  QAction *trimEOLsAct = new QAction(QIcon(":/images/trim-eols.png"), tr("Trim EOLs"), this);
  trimEOLsAct->setStatusTip(tr("Trim lines with specific EOLs"));
  trimEOLsAct->setMenu(trimEOLsMenu);

  QAction* winEolAct = new QAction(QIcon(":/images/win22.png"), tr("Windows"), this);
  winEolAct->setStatusTip(tr("Trim Windows-style line ends (CR/LF)"));
  winEolAct->setData(QsciScintilla::EolWindows);
  trimEOLsMenu->addAction(winEolAct);

  QAction* unixEolAct = new QAction(QIcon(":/images/linux22.png"), tr("Unix"), this);
  unixEolAct->setStatusTip(tr("Trim Unix-style line ends (LF)"));
  unixEolAct->setData(QsciScintilla::EolUnix);
  trimEOLsMenu->addAction(unixEolAct);

  QAction* macEolAct = new QAction(QIcon(":/images/mac22.png"), tr("Mac"), this);
  macEolAct->setStatusTip(tr("Trim Mac-style line ends (CR)"));
  macEolAct->setData(QsciScintilla::EolMac);
  trimEOLsMenu->addAction(macEolAct);


  trimToolBar = addToolBar(tr("Trim lines"));
  trimToolBar->setObjectName("trimToolBar");
  trimToolBar->setIconSize(QSize(16,16));

  trimCounter = new QSpinBox(this);
  trimCounter->setRange(1,9999);
  trimCounter->setValue(1);
  trimCounter->setSuffix(tr(" lines"));

  trimToolBar->addAction(trimAllEmptyLinesAct);
  trimToolBar->addSeparator();
  trimToolBar->addAction(trimAboveEmptyLinesAct);
  trimToolBar->addWidget(trimCounter);
  trimToolBar->addAction(trimBelowEmptyLinesAct);
  trimToolBar->addSeparator();
  trimToolBar->addAction(trimEOLsAct);
}

void MainWindow::trimAboveEmptyLines()
{
  TrimInfo ti;
  ti.count = trimCounter->value();
  ti.isAbove = true;
  textEdit->trimEmptyLines(ti);
}

void MainWindow::trimBelowEmptyLines()
{
  TrimInfo ti;
  ti.count = trimCounter->value();
  ti.isAbove = false;
  textEdit->trimEmptyLines(ti);
}

void MainWindow::trimAllEmptyLines()
{
  TrimInfo ti;
  ti.count = 0;
  ti.isAbove = true;
  textEdit->trimEmptyLines(ti);
}

void MainWindow::trimEolMenu_activated(QAction *act)
{
  textEdit->trimEndOfLines((QsciScintilla::EolMode)act->data().toInt());
}
