#include "mainwindow.h"
#include "editwindow.h"
#include "tabwidget.h"

#include <QtGui>

////////////////////////////////////////////////////////////////////////////////

QStringList MainWindow::sessionList() const
{
  INIT_SETTINGS
  settings.beginGroup("Sessions");
  return settings.childGroups();
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::saveSession()
{
  if (mySession.isEmpty())
  {
    saveSessionAs();
    return;
  }

  bool res = QMessageBox::question(this,
                                   tr("Save current session"),
                                   tr("Save current tabset as %1?").arg(mySession),
                                   QMessageBox::Ok, QMessageBox::Cancel);
  if (res == QMessageBox::Cancel)
    return;

  // save tabs
  doSaveSession(mySession);

  updateSessionActions();
}

void MainWindow::doSaveSession(const QString &name)
{
  if (name.isEmpty())
    return;

  // first, clean up old data
  doRemoveSession(name);

  // save
  INIT_SETTINGS
  settings.beginGroup("Sessions");

  doStoreTabs(settings, name);

  // update name
  mySession = name;

  statusBar()->showMessage(tr("Session %1 saved successfully").arg(name));
}

void MainWindow::saveSessionAs()
{
  QString name = QInputDialog::getText(this, tr("Save current session"),
                                       tr("Name for the session:"),
                                       QLineEdit::Normal,
                                       mySession);
  if (name.isEmpty())
    return;

  // check for an existing name
  if (sessionList().contains(name))
  {
    bool res = QMessageBox::warning(this,
                     tr("Session exists"),
                     tr("Session with name %1 exists. Do you want to overwrite it?").arg(name),
                     QMessageBox::Ok, QMessageBox::Cancel);

    if (res == QMessageBox::Cancel)
      return;
  }

  doSaveSession(name);

  updateSessionActions();
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::sessionActivated(QAction* act)
{
  doLoadSession(act->text());

  updateSessionActions();
}

void MainWindow::reloadSession()
{
  doLoadSession(mySession);

  updateSessionActions();
}

void MainWindow::doLoadSession(const QString &name)
{
  if (!maybeSave())
    return;

  closeAllWindows();

  INIT_SETTINGS
  settings.beginGroup("Sessions");

  doReadTabs(settings, name);

  if (tabWidget->count() > 1)
    tabCloseRequested(0);

  mySession = name;
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::removeSession()
{
  if (mySession.isEmpty())
    return;

  bool res = QMessageBox::warning(this,
                   tr("Delete Session"),
                   tr("Do you really want to delete session %1?").arg(mySession),
                   QMessageBox::Ok, QMessageBox::Cancel);

  if (res == QMessageBox::Cancel)
    return;

  if (!maybeSave())
    return;

  doRemoveSession(mySession);
  closeSession();
}

void MainWindow::doRemoveSession(const QString &name)
{
  INIT_SETTINGS
  settings.beginGroup("Sessions");
  settings.remove(name);
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::closeSession()
{
  closeAllWindows();
  mySession = "";

  updateSessionActions();
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::updateSessionActions()
{
  fillSessionsMenu();

  bool en = !mySession.isEmpty();
  closeSessionAct->setEnabled(en);
  removeSessionAct->setEnabled(en);
  reloadSessionAct->setEnabled(en);
  saveSessionAct->setEnabled(en);
}

void MainWindow::fillSessionsMenu()
{
  QStringList sl(sessionList());
  //qDebug() << sl;

  sessionListMenu->clear();
  foreach(QString s, sl)
  {
    QAction *act = sessionListMenu->addAction(s);
    if (mySession == s)
      sessionListMenu->setDefaultAction(act);
  }

  sessionListMenu->setEnabled(!sessionListMenu->isEmpty());
}

