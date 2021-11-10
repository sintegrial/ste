#include "mainwindow.h"
#include "editwindow.h"

#include <Qsci/qsciscintilla.h>

void MainWindow::fillBookmarkMenu()
{
  const QList<int>& list = textEdit->bookmarks();

  bookmarkMenu->clear();
  bookmarkMenu->addAction(addBookmarkAct);
  bookmarkMenu->addAction(removeAllBookmarksAct);
  removeAllBookmarksAct->setEnabled(list.count());
  bookmarkMenu->addAction(nextBookmarkAct);
  nextBookmarkAct->setEnabled(list.count());
  bookmarkMenu->addAction(prevBookmarkAct);
  prevBookmarkAct->setEnabled(list.count());

  if (list.isEmpty())
    return;

  int line1 = textEdit->firstVisibleLine();
  int line2 = line1 + textEdit->linesVisible();

  bookmarkMenu->addSeparator();

  QMap<int,int> line_id;

  for (int i = 0; i < list.count(); i++)
  {
    int id = list.at(i);
    int line = textEdit->markerLine(id);
    line_id[line] = id;
  }

  QMap<int,int>::iterator it = line_id.begin(), it_end = line_id.end();
  for (; it != it_end; it++)
  {
    int line = it.key();
    QString preview = textEdit->text(line);
    if (preview.size() > 20)
      preview = preview.left(20) + "<...>";
    QAction *act = bookmarkMenu->addAction(tr("[Line %1]    %2").arg(line+1).arg(preview));
    act->setData(it.value());

    if (line >= line1 && line <= line2)
    {
      act->setCheckable(true);
      act->setChecked(true);
    }
  }
}

void MainWindow::bookmarkActivated(QAction* act)
{
  int id = act->data().toInt();
  if (id < 0)
    return;
  textEdit->navigateBookmark(id);
}

void MainWindow::toggleBookmarks()
{
  textEdit->toggleBookmark();
  updateBookmarks();
}

void MainWindow::removeBookmarks()
{
  textEdit->removeAllBookmarks();
  updateBookmarks();
}

void MainWindow::updateBookmarks()
{
  bool cnt = textEdit->bookmarks().count();
  removeAllBookmarksAct->setEnabled(cnt);
  nextBookmarkAct->setEnabled(cnt);
  prevBookmarkAct->setEnabled(cnt);
}
