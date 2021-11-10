#include "mainwindow.h"
#include "symbolsbar.h"

void MainWindow::createSymbolsBar()
{
  symbolsBar = new SymbolsBar(this);

  QDockWidget *symbolsDock = new QDockWidget(tr("Symbols"), this);
  symbolsDock->setObjectName("SymbolsDock");
  symbolsDock->setWidget(symbolsBar);

  addDockWidget(Qt::LeftDockWidgetArea, symbolsDock);

  symbolsAct = symbolsDock->toggleViewAction();
  symbolsAct->setIcon(QIcon(":/images/character-map.png"));
  symbolsAct->setShortcut(QKeySequence("Alt+9"));
  symbolsAct->setStatusTip(tr("Show Symbols window"));
}
