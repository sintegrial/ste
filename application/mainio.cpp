#include "mainwindow.h"
#include "editwindow.h"
#include "tabwidget.h"
#include "stylemanager.h"
#include "recentfiles.h"
#include "filesdialog.h"
#include "utils.h"

#include <Qsci/qscilexer.h>
#include <Qsci/qsciscintilla.h>

#include <QUrlInfo>

////////////////////////////////////////////////////////////////////////////////

bool MainWindow::doOpen(EditWindow *te, const QString &codec)
{
  statusBar()->showMessage(tr("Opening %1...").arg(te->fileName()));

  QApplication::processEvents();
  bool res = te->load(codec);
  QApplication::processEvents();

  if (res)
    statusBar()->showMessage(tr("%1 opened successfully.").arg(te->fileName()));
  else
    statusBar()->showMessage(tr("Error opening %1.").arg(te->fileName()));

  return res;
}

//bool MainWindow::isAlreadyOpen(const QString &filename)
//{
//
//}
//
//bool MainWindow::doLoad(const QStringList &files)
//{
//  FilesDialog fd;
//  fd.setText(tr("STE - Unsaved files"),
//             tr("These files were not saved and are about to be reloaded:"),
//             tr("What do you want to do?"));
//  fd.addButton(tr("Reload from disk"), 2);
//  fd.addButton(tr("Don't reload"), 1);
//  fd.addButton(tr("Abort"), 0);
//
//}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::messageParams(const QString& msg)
{
  // show the window
  show();
  raise();

  externParams = msg.split("\n");
  if (externParams.count() <= 1)
    return;

  QTimer::singleShot(0, this, SLOT(loadFromParams()));
}

void MainWindow::loadFromParams()
{
  //QStringList list, failed;

  for (int i = 1; i < externParams.count(); i++)
  {
    QString filename = externParams.at(i);
    if (QFileInfo(filename).exists())
    {
      loadFile(filename);
//      if (loadFile(filename))
//        recentFiles->addFile(filename);


//      if (isAlreadyOpen(filename))
//      {
//       list.append(filename);
//      }
//      else
//      {
//        if (!doLoadSilent(filename))
//        {
//          failed.append(filename);
//        }
//      }
    }
  }

  //doLoad(list);

}

void MainWindow::loadFilesFromArguments()
{
//  QStringList list;

  externParams = qApp->arguments();
//
//  for (int i = 1; i < externParams.count(); i++)
//  {
//    QString filename = externParams.at(i);
//    if (QFileInfo(filename).exists())
//    {
//      list.append(filename);
//    }
//  }
//
//  doLoad(list);

  for (int i = 1; i < externParams.count(); i++)
  {
    if (loadFileSilent(externParams.at(i)))
      recentFiles->addFile(externParams.at(i));
  }
}

bool MainWindow::loadFileSilent(const QString &filename)
{
  bool newfile = true;
  QString fileName = Utils::normalizeFilename(filename);

  // check if such a file opened already
  for (int i = 0; i < tabWidget->count(); i++)
  {
    EditWindow *te = (EditWindow*) tabWidget->widget(i);
    if (te->compareFileName(fileName))
    {
      // reload file from disk
      tabWidget->setCurrentWidget(te);
      newfile = false;
      break;
    }
  }

  QFile file(fileName);
  if (!file.open(QFile::ReadOnly))
    return false;

  if (newfile)
    createTab(fileName);

  //textEdit->load();
  doOpen(textEdit);

  // assign lexer by extension
  int lt = styleManager->lexerForFilename(fileName);
  textEdit->setLexer(styleManager->createLexer(lt), lt);

  // check eols
  QsciScintilla::EolMode eols = textEdit->checkEols();
  textEdit->setEolMode(eols);

  // update
  updateFileTab(textEdit, tabWidget->currentIndex());
  updateCurrentFileStatus();

  return true;
}

EditWindow* MainWindow::findTab(const QString &fileName)
{
  for (int i = 0; i < tabWidget->count(); i++)
  {
    EditWindow *te = (EditWindow*) tabWidget->widget(i);
    if (te->compareFileName(fileName))
      return te;
  }
  return 0;
}

bool MainWindow::loadFile(const QString &filename)
{
    bool reload = false;
    QString fileName = Utils::normalizeFilename(filename);

    //qDebug() << fileName;

    // check if such a file opened already
    EditWindow *te = findTab(fileName);
    if (te)
    {
      // reload by default; ask if modified
      if (te->isModified())
      {
        QMessageBox mb(QMessageBox::Warning, tr("STE"),
                             tr("The file %1 is already open in another window.\n"
                                "Do you want to switch to this tab or reload it from disk?")
                             .arg(fileName),
                             QMessageBox::NoButton,
                             this);
        mb.addButton(tr("Switch"), QMessageBox::AcceptRole);
        mb.addButton(tr("Reload"), QMessageBox::DestructiveRole);
        mb.addButton(tr("Cancel"), QMessageBox::RejectRole);

        mb.exec();

        int role = mb.buttonRole(mb.clickedButton());
        switch (role)
        {
        case QMessageBox::RejectRole:
          // false means - does not allow continue for next files
          return false;
        case QMessageBox::AcceptRole:
          tabWidget->setCurrentWidget(te);
          return true;
        default:;
        }
      }

      // reload file from disk
      tabWidget->setCurrentWidget(te);
      reload = true;
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, tr("STE"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        // true means - allow continue for next files
        return true;
    }


    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (!reload)
    {
      if (tabWidget->count() == 1 && textEdit && textEdit->justCreated())
      {
        createTab(fileName);

        QWidget *w = tabWidget->widget(0);
        tabWidget->removeTab(0);
        delete w;
      }
      else
        createTab(fileName);
    }

    // load with default codec
    if (reload)
      //textEdit->load(textEdit->codec());
      doOpen(textEdit, textEdit->codec());
    else
      //textEdit->load();
      doOpen(textEdit);

    // assign lexer by extension
    int lt = styleManager->lexerForFilename(fileName);
    textEdit->setLexer(styleManager->createLexer(lt), lt);

    // check eols
    QsciScintilla::EolMode eols = textEdit->checkEols();
    textEdit->setEolMode(eols);

    // update
    updateFileTab(textEdit, tabWidget->currentIndex());
    updateCurrentFileStatus();

    QApplication::restoreOverrideCursor();

    statusBar()->showMessage(tr("File loaded"), 2000);

    // recent files
    recentFiles->addFile(fileName);

    return true;
}

void MainWindow::open()
{
  QString fileName = QFileDialog::getOpenFileName(
      this,
      tr("Load File"),
      textEdit->fileName().isEmpty() ? lastFileDir : textEdit->fileName()
      );

  // update the tab - a workaround
  textEdit->repaint();
  QApplication::processEvents();

  if (!fileName.isEmpty())
  {
    loadFile(fileName);
//    if (loadFile(fileName))
//      recentFiles->addFile(textEdit->fileName()); // normalized
  }
}

void MainWindow::reload()
{
  if (textEdit->justCreated())
    return;

  if (textEdit->isModified())
  {
    int ret = QMessageBox::warning(this,
                tr("STE - reload file"),
                 tr("File %1 has unsaved modifications.\n"
                     "Reloading will discard them.\n"
                    "Do you want to proceed?").arg(textEdit->fileName()),
                 QMessageBox::Yes |
                 QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
      return;
  }

  //if (textEdit->load())
  if (doOpen(textEdit))
  {
    updateFileTab(textEdit, tabWidget->currentIndex());
    updateCurrentFileStatus();
    statusBar()->showMessage(tr("File reloaded"), 2000);
  }
  else
  {
    statusBar()->showMessage(tr("File reload failed"), 2000);
  }
}

void MainWindow::reloadEncoded(QAction* act)
{
  if (textEdit->justCreated())
    return;

  const QString &codec = myCodecs.at(act->data().toInt());

  if (textEdit->isModified())
  {
    int ret = QMessageBox::warning(this,
                tr("STE - reload file as %1").arg(codec),
                 tr("File %1 has unsaved modifications.\n"
                     "Reloading will discard them.\n"
                    "Do you want to proceed?").arg(textEdit->fileName()),
                 QMessageBox::Yes |
                 QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
      return;
  }

  //if (textEdit->load(codec))
  if (doOpen(textEdit, codec))
  {
    updateFileTab(textEdit, tabWidget->currentIndex());
    updateCurrentFileStatus();
    statusBar()->showMessage(tr("File reloaded"), 2000);
  }
  else
  {
    statusBar()->showMessage(tr("File reload failed"), 2000);
  }
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  foreach(QUrl url, event->mimeData()->urls())
    if (QFileInfo(url.toLocalFile()).exists())
    {
      event->acceptProposedAction();
      return;
    }
}

void MainWindow::dropEvent ( QDropEvent * event )
{
  foreach(QUrl url, event->mimeData()->urls())
  {
    QString filename = url.toLocalFile();
    if (QFileInfo(filename).exists())
    {
      event->acceptProposedAction();

      // open it
      if (!loadFile(filename))
        return;
    }
  }
}

void MainWindow::openRecent(const QString &filename)
{
  // check if exists
  if (!QFileInfo(filename).exists())
  {
    int res = QMessageBox::warning(this,
                                   tr("File not exists"),
                                   tr("The file %1 does not exist.\n"
                                      "Do you want to remove it from recent files?").arg(filename),
                                   QMessageBox::Yes, QMessageBox::No);
    if (res == QMessageBox::Yes)
    {
      recentFiles->removeFile(filename);
    }
    return;
  }

  // if so, load
  loadFile(filename);
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::checkChangedFiles()
{
  QList<int> changed, deleted;

  for (int i = 0; i < tabWidget->count(); i++)
  {
    EditWindow *te = (EditWindow*) tabWidget->widget(i);
    if (te->justCreated())
      continue;

    QString name = te->fullName();

    QFileInfo f(name);
    if (!f.exists())
    {
      deleted.append(i);
      continue;
    }

    if (f.lastModified() > te->lastModified())
    {
      changed.append(i);
    }

    // check RO attribute as well
    bool ro = !f.isWritable();
    if (ro != te->isReadOnly())
    {
      te->setReadOnly(ro);
      updateFileTabIcon(te, i);
    }
  }

  if (deleted.count())
  {
    FilesDialog fd(this);
    fd.setText(tr("STE - Removed files"),
               tr("These files were removed from file system"),
               tr("Do you want to keep them in the editor?"));
    fd.addButton(tr("Keep"), 1);
    fd.addButton(tr("Don't keep"), 0);

    for (int i = 0; i < deleted.count(); i++)
    {
      int idx = deleted.at(i);
      EditWindow *te = (EditWindow*) tabWidget->widget(idx);
      fd.addItem(te->fullName());
    }

    if (fd.exec() == 0)
    {
      for (int i = deleted.count()-1; i >= 0; i--)
      {
        int idx = deleted.at(i);
        EditWindow *te = (EditWindow*) tabWidget->widget(idx);
        tabWidget->setCurrentIndex(idx);
        disconnectSignals();
        tabWidget->removeTab(idx);
        delete te;
      }

      if (!tabWidget->count())
        createTab("");
    }

  }

  if (changed.count())
  {
    FilesDialog fd(this);
    fd.setText(tr("STE - Changed files"),
               tr("These files were changed on file system"),
               tr("Do you want to reload them?"));
    fd.addButton(tr("Reload"), 1);
    fd.addButton(tr("Don't reload"), 0);

    for (int i = 0; i < changed.count(); i++)
    {
      int idx = changed.at(i);
      EditWindow *te = (EditWindow*) tabWidget->widget(idx);
      fd.addItem(te->fullName());
    }

    if (fd.exec() == 1)
    {
      for (int i = 0; i < changed.count(); i++)
      {
        int idx = changed.at(i);
        EditWindow *te = (EditWindow*) tabWidget->widget(idx);
        //te->load();
        doOpen(te);

        updateFileTab(te, idx);
        if (idx == tabWidget->currentIndex())
          updateCurrentFileStatus();
      }
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

bool MainWindow::doSaveAll(int skip, bool silent)
{
  FilesDialog fd(this);
  fd.setText(tr("STE - Unsaved files"),
             tr("These files were not saved:"),
             tr("Do you want to continue?"));
  fd.addButton(tr("Continue"), 1);
  fd.addButton(tr("Abort"), 0);


  int fails = 0;
  int saves = 0;
  int total = tabWidget->count();

  //QStringList failedNames;

  statusBar()->showMessage(tr("Saving in progress..."));

  for (int i = 0; i < total; i++)
  {
    if (skip == i)
      continue;

    EditWindow *edit = (EditWindow*) tabWidget->widget(i);
    if (!edit->isModified())
      continue;

    if (edit->justCreated())
    {
      // make tab current
      tabWidget->setCurrentIndex(i);

      // ask for the name
      QString fileName = QFileDialog::getSaveFileName(this, tr("Save File as..."));

      // update the tab - a workaround
      textEdit->repaint();
      QApplication::processEvents();

      if (fileName.isEmpty())
      {
//        statusBar()->showMessage(tr("Save cancelled"));
//        return false;
        fails++;
        fd.addItem(edit->fullName());
        continue;
      }

      if (edit->save(fileName))
      {
        saves++;

        updateFileSavedState(edit, i);
      }
      else
      {
        //failedNames.append(fileName);
        fails++;
        fd.addItem(edit->fullName());
      }

      continue;
    }

    if (edit->save())
    {
      saves++;
      updateFileSavedState(edit, i);
    }
    else
    {
      //failedNames.append(edit->fullName());
      fails++;
      fd.addItem(edit->fullName());
    }

  }

  if (fails)
  {
    statusBar()->showMessage(tr("%1 file(s) saved, %2 file(s) failed").arg(saves).arg(fails));
    if (silent)
      return false;

    int res = fd.exec();
    return (res == 1); // true if continue, false otherwise
  }

  statusBar()->showMessage(tr("%1 file(s) saved").arg(saves));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool MainWindow::save()
{
  if (!textEdit)
    return false;

  if (textEdit->justCreated())
  {
    return saveAs();
  } else {
    return saveFile(textEdit->fileName());
  }
}

bool MainWindow::saveAll()
{
  return doSaveAll();
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save File as..."),
        textEdit->fileName().isEmpty() ? lastFileDir : textEdit->fileName()
        );

    // update the tab - a workaround
    textEdit->repaint();
    QApplication::processEvents();

    if (fileName.isEmpty())
        return false;

    bool ok = saveFile(fileName);
    if (ok)
      recentFiles->addFile(textEdit->fileName()); // use normalized one

    // assign lexer by extension
    int lt = styleManager->lexerForFilename(fileName);
    if (lt != textEdit->lexerType())
    {
      textEdit->setLexer(styleManager->createLexer(lt), lt);
      styleManager->setCurrentLexer(lt);
    }

    return ok;
}

bool MainWindow::saveFile(const QString &filename)
{
  QString fileName = Utils::normalizeFilename(filename);

  if (!textEdit->save(fileName))
  {
    bool fail = false;

    // check for read only
    if (!QFile(fileName).isWritable())
    {
      // switch to RO mode ?
      textEdit->setReadOnly(true);
      int idx = tabWidget->currentIndex();
      updateFileTabIcon(textEdit, idx);


      int res = QMessageBox::warning(this, tr("STE - File is Read Only"),
                           tr("Cannot write file %1 because it is not writable.\n"
                              "Do you want to try forced writing?")
                           .arg(fileName),
                           QMessageBox::Save, QMessageBox::Cancel);

      if (res == QMessageBox::Cancel)
        return false;

      // try force writing
      setReadonly(false);

      if (!textEdit->save(fileName))
        fail = true;
    }

    if (fail)
    {
      QMessageBox::warning(this, tr("STE - Saving failed"),
                           tr("Cannot write file %1.")
                           .arg(fileName));
      return false;
    }
  }

  //textEdit->setModified(false);
  textEdit->setFilename(fileName);

  int idx = tabWidget->currentIndex();

  //currentTabChanged(idx);
  updateFileSavedState(textEdit, idx);

  statusBar()->showMessage(tr("File saved"), 2000);
  return true;
}

bool MainWindow::maybeSave(int skip)
{
  FilesDialog fd(this);
  fd.setText(tr("STE - Unsaved files"),
             tr("These files were modified but not saved:"),
             tr("Do you want to save changes?"));
  fd.addButton(tr("Save All && Continue"), 2, true);
  fd.addButton(tr("Don't save && Continue"), 1);
  fd.addButton(tr("Abort"), 0);

  //bool dontask = false;

  for (int i = 0; i < tabWidget->count(); i++)
  {
    EditWindow *edit = (EditWindow*) tabWidget->widget(i);
    if (edit->isModified())
    {
      fd.addItem(edit->fullName());
    }
  }

  if (fd.itemCount())
  {
    int res = fd.exec();
    if (res == 0)
      return false;
    if (res == 1)
      return true;
  }

  return doSaveAll(skip, false);

  /*
  for (int i = 0; i < tabWidget->count(); i++)
  {
    if (i == skip)
      continue;

    textEdit = (EditWindow*) tabWidget->widget(i);

    if (textEdit->isModified())
    {
      if (dontask)
      {
        save();
        continue;
      }

        tabWidget->setCurrentWidget(textEdit);

        int ret = QMessageBox::warning(this,
                    tr("STE - closing %1").arg(textEdit->fileName()),
                     tr("The document %1 has been modified.\n"
                        "Do you want to save your changes?").arg(textEdit->fileName()),
                     QMessageBox::Yes |
                     QMessageBox::YesToAll |
                     QMessageBox::No |
                     QMessageBox::NoToAll |
                     QMessageBox::Cancel);

        switch (ret)
        {
          case QMessageBox::YesToAll:
            dontask = true;
          case QMessageBox::Yes:
            save();
            break;
          case QMessageBox::NoToAll:
            textEdit = (EditWindow*) tabWidget->currentWidget();
            return true;
          case QMessageBox::Cancel:
            textEdit = (EditWindow*) tabWidget->currentWidget();
            return false;
        }
      }
  }

  textEdit = (EditWindow*) tabWidget->currentWidget();

  return true;
  */
}


void MainWindow::saveEncoded(QAction* act)
{
  const QString &codec = myCodecs.at(act->data().toInt());

  int ret = QMessageBox::warning(this,
              tr("STE - saving as %1").arg(codec),
               tr("%1\n"
                   "Saving with different encoding may harm file contents.\n"
                  "Do you want to proceed?").arg(textEdit->fileName()),
               QMessageBox::Yes |
               QMessageBox::Cancel);
  if (ret == QMessageBox::Cancel)
    return;

  if (textEdit->saveEncoded(codec))
  {
    statusBar()->showMessage(tr("File saved with new encoding"), 2000);

    codecChanged();
    documentChanged();
  }
  else
  {
    statusBar()->showMessage(tr("File encoding failed"), 2000);
  }
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::setReadonly(bool set)
{
  // try to change attribute
  if (Utils::setFileReadonly(textEdit->fileName(), set) <= 0)
  {
    // error
    statusBar()->showMessage(tr("Cannot change Read Only attribute"), 2000);

    QMessageBox::critical(this, tr("Error"),
                          tr("Cannot change Read Only attribute")
                          );
  }

  textEdit->setReadOnly(set);

  int idx = tabWidget->currentIndex();
  updateFileTabIcon(textEdit, idx);

  if (textEdit->justCreated())
    return;
}
