#include "recentfiles.h"

#define MAX_COUNT 10

RecentFiles::RecentFiles(QWidget *parent) :
    QObject(parent),
    maxCount(MAX_COUNT)
{
  recentMenu = new QMenu(parent);
  connect(recentMenu, SIGNAL(triggered(QAction*)), this, SLOT(chosen(QAction*)));
}

void RecentFiles::addFile(const QString &filename)
{
  if (recentActions.contains(filename))
    return;

  recentActions.prepend(filename);
  while (recentActions.count() >= maxCount)
    recentActions.removeLast();
  updateMenu();
}

void RecentFiles::removeFile(const QString &filename)
{
  if (!recentActions.contains(filename))
    return;

  recentActions.removeAll(filename);
  updateMenu();
}

void RecentFiles::updateMenu()
{
  recentMenu->clear();
  foreach (QString name, recentActions)
  {
    recentMenu->addAction(name);
  }
}

void RecentFiles::setMaxFileCount(int max)
{
  maxCount = max <= 0 ? MAX_COUNT : max;
  if (max >= recentActions.count())
    return;

  while (recentActions.count() >= maxCount)
    recentActions.removeLast();
  updateMenu();
}

void RecentFiles::chosen(QAction* act)
{
  emit activated(act->text());
}

void RecentFiles::readConfig(QSettings &set)
{
  int cnt = set.beginReadArray("RecentFiles");
  maxCount = set.value("maxCount", maxCount).toInt();
  for (int i = 0; i < cnt; i++)
  {
    set.setArrayIndex(i);
    recentActions.append(set.value("file").toString());
  }
  set.endArray();

  updateMenu();
}

void RecentFiles::writeConfig(QSettings &set)
{
  set.beginWriteArray("RecentFiles", recentActions.count());
  set.setValue("maxCount", maxCount);
  for (int i = 0; i < recentActions.count(); i++)
  {
    set.setArrayIndex(i);
    set.setValue("file", recentActions.at(i));
  }
  set.endArray();
}
