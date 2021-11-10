#include "mainwindow.h"
#include "editwindow.h"
#include "explorerbar.h"

////////////////////////////////////////////////////////////////////////////////

void MainWindow::createExplorerBar()
{
  explorerBar = new ExplorerBar(this);
  connect(explorerBar, SIGNAL(fileClicked(const QString&)),
          this, SLOT(loadFile(const QString&)));
  connect(explorerBar, SIGNAL(fileCurrent(const QString&)),
          statusBar(), SLOT(showMessage(const QString&)));
  connect(explorerBar, SIGNAL(synchFileRequest()), this, SLOT(exploreFile()));

  QDockWidget *explorerDock = new QDockWidget(tr("File Explorer"), this);
  explorerDock->setObjectName("ExplorerDock");
  explorerDock->setWidget(explorerBar);

  addDockWidget(Qt::LeftDockWidgetArea, explorerDock);

  explorerAct = explorerDock->toggleViewAction();
  explorerAct->setIcon(QIcon(":/images/view-tree.png"));
  explorerAct->setShortcut(QKeySequence("Alt+0"));
  explorerAct->setStatusTip(tr("Show File Explorer window"));
}

void MainWindow::exploreFile()
{
  if (textEdit->justCreated())
    return;

  explorerBar->navigate(textEdit->fileName());

  if (!explorerAct->isChecked())
    explorerAct->activate(QAction::Trigger);
}
