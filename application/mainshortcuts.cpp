#include "mainwindow.h"
#include "shortcutbar.h"

void MainWindow::createShortcurBar()
{
  shortcutBar = new ShortcutBar(this);
  addSideBar(shortcutBar);

  QList<QAction*> acts;

  // IO actions
//  acts << newAct << openAct << reloadAct << saveAct << saveAllAct
//      << saveAsAct << readOnlyAct;
//  acts << bomAct;
//
//  shortcutBar->addItems(acts);

  shortcutBar->addItems(fileMenu);

  shortcutBar->addItems(editMenu);

  shortcutBar->addItems(formatMenu);

  shortcutBar->addItems(viewMenu);

  shortcutBar->addItems(searchMenu);

  shortcutBar->addItems(bookmarkMenu);

  shortcutBar->addItems(markupMenu);

  shortcutBar->addItems(sessionMenu);

  shortcutBar->addItems(toolsMenu);

  shortcutBar->addItems(helpMenu);

}

void MainWindow::showShortcuts()
{
  showSideBar(shortcutBar);
}
