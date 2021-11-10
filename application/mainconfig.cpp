#include "mainwindow.h"
#include "editwindow.h"
#include "tabwidget.h"
#include "config.h"
#include "recentfiles.h"
#include "explorerbar.h"
#include "sidebarbase.h"
#include "translationmanager.h"
#include "previewmanager.h"
#include "stylemanager.h"
#include "symbolsbar.h"
#include "wrapdialog.h"

void MainWindow::readSettings()
{
    INIT_SETTINGS;
    restoreState(settings.value("state").toByteArray());
    restoreGeometry(settings.value("geometry").toByteArray());

    zoom = settings.value("zoom", BASE_ZOOM).toInt();
    zoomInAct->setEnabled(zoom < MAX_ZOOM);
    zoomOutAct->setEnabled(zoom > 1);

    whiteSpacesAct->setChecked(settings.value("whitespaces", false).toBool());
    eolsAct->setChecked(settings.value("eols", false).toBool());
    indentGuidesAct->setChecked(settings.value("indentGuides", false).toBool());
    wrapAct->setChecked(settings.value("wrap", false).toBool());

    // config
    config->readConfig(settings);

    // recents
    recentFiles->readConfig(settings);

    // explorer
    explorerBar->readConfig(settings);

    // symbols
    symbolsBar->readConfig(settings);

    // style manager
    styleManager->readConfig(settings);
    //StyleManager::instance()->loadCurrentStyle(config->colorScheme);

    // preview manager
    previewManager->readConfig(settings);

    // wrap dialog
    wrapDialog->readConfig(settings);

    // bars
    for (int i = 0; i < sideBars.count(); i++)
      sideBars.at(i)->readConfig(settings);

    // tabs
    doReadTabs(settings, "Files");

    // session
    mySession = settings.value("session").toString();
    updateSessionActions();
}

void MainWindow::doReadTabs(QSettings &settings, const QString &name)
{
    tabWidget->blockSignals(true);

    int count = settings.beginReadArray(name);
    for (int i = 0; i < count; i++)
    {
      settings.setArrayIndex(i);

      QString fileName = settings.value("file").toString();
      createTab(fileName);

      QFile file(fileName);
      if (!file.open(QFile::ReadOnly)) {
        continue;
      }

      QString codec = settings.value("codec").toString();

      // load with stored codec
      //textEdit->load(codec);
      doOpen(textEdit, codec);

      // check eols
      QsciScintilla::EolMode eols = textEdit->checkEols();
      textEdit->setEolMode(eols);

      // update
      updateFileTab(textEdit, tabWidget->currentIndex());
      updateCurrentFileStatus();

      // read bookmarks
      QStringList booklines = settings.value("bookmarks").toStringList();
      textEdit->setBookmarks(booklines);

      // read lexer id
      QString lexerSID = settings.value("lexer").toString();
      if (lexerSID == "none")
        continue;

      // assign lexer by name
      int lt = styleManager->lexerOfSID(lexerSID);
      QsciLexer *lexer = styleManager->createLexer(lt);
      if (lexer)
      {
        textEdit->setLexer(lexer, lt);
        continue;
      }

      // else, assign lexer by extension
      lt = styleManager->lexerForFilename(fileName);
      lexer = styleManager->createLexer(lt);
      textEdit->setLexer(lexer, lt);
    }
    settings.endArray();

    int idx = settings.value("currentTab", 0).toInt();
    tabWidget->setCurrentIndex(idx);

    tabWidget->blockSignals(false);

    currentTabChanged(idx);

    if (!tabWidget->count())
      newFile();
}

void MainWindow::writeSettings()
{
    INIT_SETTINGS
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());

    settings.setValue("zoom", zoom);
    settings.setValue("whitespaces", whiteSpacesAct->isChecked());
    settings.setValue("eols", eolsAct->isChecked());
    settings.setValue("indentGuides", indentGuidesAct->isChecked());
    settings.setValue("wrap", wrapAct->isChecked());

    // config
    config->writeConfig(settings);

    // recents
    recentFiles->writeConfig(settings);

    // explorer
    explorerBar->writeConfig(settings);

    // symbols
    symbolsBar->writeConfig(settings);

    // style manager
    styleManager->writeConfig(settings);

    // preview manager
    previewManager->writeConfig(settings);

    // wrap dialog
    wrapDialog->writeConfig(settings);

    // bars
    for (int i = 0; i < sideBars.count(); i++)
      sideBars.at(i)->writeConfig(settings);

    // tabs
    doStoreTabs(settings, "Files");
    settings.setValue("currentTab", tabWidget->currentIndex());

    // session
    settings.setValue("session", mySession);
}

void MainWindow::doStoreTabs(QSettings &settings, const QString &name)
{
  settings.beginWriteArray(name, tabWidget->count());
  for (int i = 0; i < tabWidget->count(); i++)
  {
    EditWindow *ew = (EditWindow*) tabWidget->widget(i);
    settings.setArrayIndex(i);
    settings.setValue("file", ew->fileName());
    settings.setValue("codec", ew->codec());
    settings.setValue("lexer", styleManager->lexerSID(ew->lexerType()));
    settings.setValue("bookmarks", ew->bookmarkedLinesToStringList());
  }
  settings.endArray();
}

