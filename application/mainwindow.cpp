#include <QtGui>
#include <Qsci/qsciprinter.h>

#include <iconv.h>

#include "clicklabel.h"

#include "mainwindow.h"
#include "editwindow.h"
#include "tabwidget.h"

#include "config.h"
#include "configdialog.h"
#include "recentfiles.h"

#include "stylemanager.h"
#include "formatmanager.h"
#include "toolmanager.h"
#include "toolmanagerdialog.h"
#include "previewmanager.h"

#include "sidebarbase.h"
#include "explorerbar.h"
#include "wordformatbar.h"
#include "symbolsbar.h"

#include "defines.h"

MainWindow::MainWindow() : QMainWindow(),
    textEdit(0),
    zoom(BASE_ZOOM)
{
    defineCodecs();

    setAcceptDrops(true);
    setWindowIcon(QIcon(":/images/icon.png"));

    createMainWidget();
    createTabBar();
    createSearchBar();
    createExplorerBar();
    createWordFormatBar();
    createSymbolsBar();
    createNumbersBar();
    createWrapDialog();

    recentFiles = new RecentFiles(this);
    recentFiles->setMaxFileCount(30);
    recentFiles->menu()->setTitle(tr("Recent files"));
    connect(recentFiles, SIGNAL(activated(const QString&)), this, SLOT(openRecent(const QString&)));

    createManagers();

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    // config
    config = new Config();
    ConfigInit init;
    init.codecs = &myCodecs;
    init.eols = eolsGroup->actions();
    configDialog = new ConfigDialog(init, this);
    connect(configDialog, SIGNAL(configChanged()), this, SLOT(applyConfig()));

    // printer
    printer = new QsciPrinter;
    printDialog = new QPrintDialog(printer, this);

    // shortcut bar before
    createShortcurBar();

    // style
#ifdef Q_WS_WIN
    QString guiPath = QCoreApplication::applicationDirPath() + "/files/gui/";
#else
    QString guiPath = "/usr/share/sintegrial/ste/files/gui/";
#endif
    QFile guiFile(guiPath + "modern/style.qss");
    if (guiFile.open(QFile::ReadOnly))
    {
      QApplication::setStyle("plastique");
      QString css(guiFile.readAll());
      qApp->setStyleSheet(css);
    }
}

void MainWindow::start()
{
  show();

  // load last state
  readSettings();

  // connect focus changed handler
  connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)),
          this, SLOT(appFocusChanged(QWidget*,QWidget*)));
  qApp->installEventFilter(this);

//  connect(qApp, SIGNAL(aboutToQuit()),
//          this, SLOT(sideBarClosed()));

  // load arg files
  loadFilesFromArguments();
}

void MainWindow::createMainWidget()
{
  mainSplitter = new QSplitter(Qt::Vertical, this);
  mainSplitter->setChildrenCollapsible(false);
  setCentralWidget(mainSplitter);

  mainWidget = new QWidget(this);
  mainWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
  QVBoxLayout *vbl = new QVBoxLayout();
  vbl->setContentsMargins(0,0,0,0);
  vbl->setMargin(0);
  vbl->setSpacing(1);
  mainWidget->setLayout(vbl);

  mainSplitter->addWidget(mainWidget);

  barStack = new QStackedWidget(this);
  barStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  barStack->hide();

  mainSplitter->addWidget(barStack);
}

MainWindow::~MainWindow()
{
  delete printer;
  delete config;
}

////////////////////////////////////////////////////////////////////////////////

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
  bool b = QObject::eventFilter(obj, event);

  if (event->type() == QEvent::ApplicationActivate)
  {
    QTimer::singleShot(100, this, SLOT(checkChangedFiles()));
    raise();
    return true;
  }

  if (event->type() == QEvent::ContextMenu)
  {
    if (dynamic_cast<EditWindow*>(obj->parent()) == textEdit)
    {
      // handle context menu
      editContextMenu->popup(textEdit->mapToGlobal(((QContextMenuEvent*)event)->pos()));
      return true;
    }
  }

  // standard event processing
  return b;
}

void MainWindow::appFocusChanged(QWidget * /*old*/, QWidget * /*now*/)
{
//  qDebug() << "focus";
//  qDebug() << this;
//  qDebug() << now;
//  qDebug() << old;
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
  if (event->modifiers() & Qt::ControlModifier)
  {
    if (event->delta() > 0)
    {
      zoomInAct->activate(QAction::Trigger);
      event->accept();
      return;
    }

    if (event->delta() < 0)
    {
      zoomOutAct->activate(QAction::Trigger);
      event->accept();
      return;
    }
  }

  QMainWindow::wheelEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        sideBarClosed();
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::connectSignals()
{
  if (!textEdit)
    return;

  connect(textEdit, SIGNAL(modificationChanged(bool)), this, SLOT(documentWasModified(bool)));
  connect(textEdit, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(cursorPositionChanged(int, int)));
  connect(textEdit, SIGNAL(linesChanged()), this, SLOT(linesChanged()));
  connect(textEdit, SIGNAL(textChanged()), this, SLOT(documentChanged()));
  connect(textEdit, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

  connect(textEdit, SIGNAL(marginClicked(int,int,Qt::KeyboardModifiers)),
          this, SLOT(marginClicked(int,int,Qt::KeyboardModifiers)));

  connect(textEdit, SIGNAL(copyAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
  connect(textEdit, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));

  connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));
  connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));
  connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

  connect(undoAct, SIGNAL(triggered()), textEdit, SLOT(undo()));
  connect(redoAct, SIGNAL(triggered()), textEdit, SLOT(redo()));

  connect(eolsAct, SIGNAL(toggled(bool)), textEdit, SLOT(setEolVisibility(bool)));
  connect(eolsGroup, SIGNAL(triggered(QAction*)), this, SLOT(setEolsAction(QAction*)));
  connect(indentGuidesAct, SIGNAL(toggled(bool)), textEdit, SLOT(setIndentationGuides(bool)));

  connect(foldAct, SIGNAL(triggered()), textEdit, SLOT(foldAll()));
  connect(foldExpandAct, SIGNAL(triggered()), textEdit, SLOT(foldExpandAll()));
  connect(foldContractAct, SIGNAL(triggered()), textEdit, SLOT(foldContractAll()));

  connect(upperAct, SIGNAL(triggered()), textEdit, SLOT(uppercaseSelectedText()));
  connect(lowerAct, SIGNAL(triggered()), textEdit, SLOT(lowercaseSelectedText()));
  connect(lineDuplicateAct, SIGNAL(triggered()), textEdit, SLOT(duplicateCurrentLine()));
  connect(wordKillAct, SIGNAL(triggered()), textEdit, SLOT(removeWord()));
  connect(lineUpAct, SIGNAL(triggered()), textEdit, SLOT(moveCurrentLineUpper()));
  connect(lineDownAct, SIGNAL(triggered()), textEdit, SLOT(moveCurrentLineLower()));
  connect(lineFillAct, SIGNAL(triggered()), textEdit, SLOT(fillupCurrentLine()));

  connect(tabsToSpacesAct, SIGNAL(triggered()), textEdit, SLOT(tabsToSpaces()));
  connect(spacesToTabsAct, SIGNAL(triggered()), textEdit, SLOT(spacesToTabs()));
  connect(trimRightSpacesAct, SIGNAL(triggered()), textEdit, SLOT(trimLastSpaces()));
  connect(trimSpacesAct, SIGNAL(triggered()), textEdit, SLOT(trimSpaces()));

  connect(wordFormatBar, SIGNAL(format(FormatInfo&)), textEdit, SLOT(formatWords(FormatInfo&)));

  connect(nextBookmarkAct, SIGNAL(triggered()), textEdit, SLOT(nextBookmark()));
  connect(prevBookmarkAct, SIGNAL(triggered()), textEdit, SLOT(prevBookmark()));

  connect(searchBraceAct, SIGNAL(triggered()), textEdit, SLOT(findMatchingBrace()));
  connect(selectBraceAct, SIGNAL(triggered()), textEdit, SLOT(selectToMatchingBrace()));

  connect(symbolsBar, SIGNAL(insertSymbol(QString)), textEdit, SLOT(insertBefore(QString)));
}

void MainWindow::disconnectSignals()
{
  if (!textEdit)
    return;

  disconnect(textEdit, SIGNAL(modificationChanged(bool)), this, SLOT(documentWasModified(bool)));
  disconnect(textEdit, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(cursorPositionChanged(int, int)));
  disconnect(textEdit, SIGNAL(linesChanged()), this, SLOT(linesChanged()));
  disconnect(textEdit, SIGNAL(textChanged()), this, SLOT(documentChanged()));
  disconnect(textEdit, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

  disconnect(textEdit, SIGNAL(marginClicked(int,int,Qt::KeyboardModifiers)),
          this, SLOT(marginClicked(int,int,Qt::KeyboardModifiers)));

  disconnect(textEdit, SIGNAL(copyAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
  disconnect(textEdit, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));

  disconnect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));
  disconnect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));
  disconnect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

  disconnect(undoAct, SIGNAL(triggered()), textEdit, SLOT(undo()));
  disconnect(redoAct, SIGNAL(triggered()), textEdit, SLOT(redo()));

  disconnect(eolsAct, SIGNAL(toggled(bool)), textEdit, SLOT(setEolVisibility(bool)));
  disconnect(eolsGroup, SIGNAL(triggered(QAction*)), this, SLOT(setEolsAction(QAction*)));
  disconnect(indentGuidesAct, SIGNAL(toggled(bool)), textEdit, SLOT(setIndentationGuides(bool)));

  disconnect(foldAct, SIGNAL(triggered()), textEdit, SLOT(foldAll()));
  disconnect(foldExpandAct, SIGNAL(triggered()), textEdit, SLOT(foldExpandAll()));
  disconnect(foldContractAct, SIGNAL(triggered()), textEdit, SLOT(foldContractAll()));

  disconnect(upperAct, SIGNAL(triggered()), textEdit, SLOT(uppercaseSelectedText()));
  disconnect(lowerAct, SIGNAL(triggered()), textEdit, SLOT(lowercaseSelectedText()));
  disconnect(lineDuplicateAct, SIGNAL(triggered()), textEdit, SLOT(duplicateCurrentLine()));
  disconnect(wordKillAct, SIGNAL(triggered()), textEdit, SLOT(removeWord()));
  disconnect(lineUpAct, SIGNAL(triggered()), textEdit, SLOT(moveCurrentLineUpper()));
  disconnect(lineDownAct, SIGNAL(triggered()), textEdit, SLOT(moveCurrentLineLower()));
  disconnect(lineFillAct, SIGNAL(triggered()), textEdit, SLOT(fillupCurrentLine()));

  disconnect(tabsToSpacesAct, SIGNAL(triggered()), textEdit, SLOT(tabsToSpaces()));
  disconnect(spacesToTabsAct, SIGNAL(triggered()), textEdit, SLOT(spacesToTabs()));
  disconnect(trimRightSpacesAct, SIGNAL(triggered()), textEdit, SLOT(trimLastSpaces()));
  disconnect(trimSpacesAct, SIGNAL(triggered()), textEdit, SLOT(trimSpaces()));

  disconnect(wordFormatBar, SIGNAL(format(FormatInfo&)), textEdit, SLOT(formatWords(FormatInfo&)));

  disconnect(nextBookmarkAct, SIGNAL(triggered()), textEdit, SLOT(nextBookmark()));
  disconnect(prevBookmarkAct, SIGNAL(triggered()), textEdit, SLOT(prevBookmark()));

  disconnect(searchBraceAct, SIGNAL(triggered()), textEdit, SLOT(findMatchingBrace()));
  disconnect(selectBraceAct, SIGNAL(triggered()), textEdit, SLOT(selectToMatchingBrace()));

  disconnect(symbolsBar, SIGNAL(insertSymbol(QString)), textEdit, SLOT(insertBefore(QString)));
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::createManagers()
{
  styleManager = new StyleManager(this);
  connect(styleManager, SIGNAL(activated(int)), this, SLOT(lexerChanged(int)));

  formatManager = new FormatManager(this);

  toolManager = new ToolManager(this);
  toolManagerDialog = new ToolManagerDialog(this, toolManager);

  previewManager = new PreviewManager(this);
}

void MainWindow::createActions()
{
    newAct = new QAction(QIcon(":/images/file-new.png"), tr("&New"), this);
    newAct->setShortcut(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/images/file-open.png"), tr("&Open..."), this);
    openAct->setShortcut(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    reloadAct = new QAction(QIcon(":/images/file-reload.png"),tr("&Reload"), this);
    reloadAct->setShortcut(QKeySequence("F5"));
    reloadAct->setStatusTip(tr("Reload file"));
    connect(reloadAct, SIGNAL(triggered()), this, SLOT(reload()));

    saveAct = new QAction(QIcon(":/images/file-save.png"), tr("&Save"), this);
    saveAct->setShortcut(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAllAct = new QAction(QIcon(":/images/file-saveall.png"), tr("Save all"), this);
    saveAllAct->setShortcut(tr("Ctrl+Shift+S"));
    saveAllAct->setStatusTip(tr("Save all the documents to disk"));
    connect(saveAllAct, SIGNAL(triggered()), this, SLOT(saveAll()));

    saveAsAct = new QAction(QIcon(":/images/file-saveas.png"), tr("Save &as..."), this);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document to disk under new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    readOnlyAct = new QAction(tr("Read Only"), this);
    readOnlyAct->setStatusTip(tr("Change file's Read Only flag"));
    readOnlyAct->setCheckable(true);
    connect(readOnlyAct, SIGNAL(toggled(bool)), this, SLOT(setReadonly(bool)));

    bomAct = new QAction(tr("Write BOM"), this);
    bomAct->setStatusTip(tr("Write Byte Order Mark signature for Unicode files"));
    bomAct->setCheckable(true);
    connect(bomAct, SIGNAL(toggled(bool)), this, SLOT(bomChanged(bool)));

    printAct = new QAction(QIcon(":/images/file-print.png"), tr("&Print..."), this);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setStatusTip(tr("Print the document or its part"));
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    exitAct = new QAction(tr("E&xit"), this);
    //exitAct->setShortcut(QKeySequence::Quit);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));



    cutAct = new QAction(QIcon(":/images/editcut.png"), tr("Cu&t"), this);
    cutAct->setShortcut(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));

    copyAct = new QAction(QIcon(":/images/editcopy.png"), tr("&Copy"), this);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));

    pasteAct = new QAction(QIcon(":/images/editpaste.png"), tr("&Paste"), this);
    pasteAct->setShortcut(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));

    undoAct = new QAction(QIcon(":/images/edit-undo.png"), tr("&Undo"), this);
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Discard latest actions"));

    redoAct = new QAction(QIcon(":/images/edit-redo.png"), tr("&Redo"), this);
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setStatusTip(tr("Repeat latest discarded actions"));


    upperAct = new QAction(tr("UPPERCASE"), this);
    upperAct->setShortcut(tr("Ctrl+U"));
    upperAct->setStatusTip(tr("Turn selected text to uppercase"));

    lowerAct = new QAction(tr("lowercase"), this);
    lowerAct->setShortcut(tr("Ctrl+Shift+U"));
    lowerAct->setStatusTip(tr("Turn selected text to lowercase"));

    lineDuplicateAct = new QAction(tr("Duplicate line/selection"), this);
    lineDuplicateAct->setShortcut(tr("Ctrl+D"));
    lineDuplicateAct->setStatusTip(tr("Duplicate current line or selection"));

    lineKillAct = new QAction(tr("Remove line"), this);
    lineKillAct->setShortcut(tr("Ctrl+L"));
    lineKillAct->setData(Qt::Key_L | Qt::ControlModifier);
    lineKillAct->setStatusTip(tr("Remove current line"));
    connect(lineKillAct, SIGNAL(triggered()), this, SLOT(sendActionKeys()));

    lineEndKillAct = new QAction(tr("Remove to end of line"), this);
    lineEndKillAct->setShortcut(tr("Ctrl+Shift+Del"));
    lineEndKillAct->setData(Qt::Key_Delete | Qt::ControlModifier | Qt::ShiftModifier);
    lineEndKillAct->setStatusTip(tr("Remove text from cursor to the end of current line"));
    connect(lineEndKillAct, SIGNAL(triggered()), this, SLOT(sendActionKeys()));

    lineStartKillAct = new QAction(tr("Remove to beginning of line"), this);
    lineStartKillAct->setShortcut(tr("Ctrl+Shift+Backspace"));
    lineStartKillAct->setData(Qt::Key_Backspace | Qt::ControlModifier | Qt::ShiftModifier);
    lineStartKillAct->setStatusTip(tr("Remove text from cursor to the beginning of current line"));
    connect(lineStartKillAct, SIGNAL(triggered()), this, SLOT(sendActionKeys()));

    wordEndKillAct = new QAction(tr("Remove to end of word"), this);
    wordEndKillAct->setShortcut(tr("Ctrl+Del"));
    wordEndKillAct->setData(Qt::Key_Delete | Qt::ControlModifier);
    wordEndKillAct->setStatusTip(tr("Remove text from cursor to the end of current word"));
    connect(wordEndKillAct, SIGNAL(triggered()), this, SLOT(sendActionKeys()));

    wordStartKillAct = new QAction(tr("Remove to beginning of word"), this);
    wordStartKillAct->setShortcut(Qt::Key_Backspace | Qt::ControlModifier);
    wordStartKillAct->setData(Qt::Key_Backspace | Qt::ControlModifier);
    wordStartKillAct->setStatusTip(tr("Remove text from cursor to the beginning of current word"));
    connect(wordStartKillAct, SIGNAL(triggered()), this, SLOT(sendActionKeys()));

    wordKillAct = new QAction(tr("Remove word"), this);
    wordKillAct->setShortcut(tr("Alt+Del"));
    wordKillAct->setStatusTip(tr("Remove current word"));

    lineSwapAct = new QAction(tr("Swap lines"), this);
    lineSwapAct->setShortcut(tr("Ctrl+T"));
    lineSwapAct->setData(Qt::Key_T | Qt::ControlModifier);
    lineSwapAct->setStatusTip(tr("Swap current and upper lines"));
    connect(lineSwapAct, SIGNAL(triggered()), this, SLOT(sendActionKeys()));

    lineUpAct = new QAction(tr("Line up"), this);
    lineUpAct->setShortcut(tr("Ctrl+Shift+Up"));
    lineUpAct->setStatusTip(tr("Move current line upwards"));

    lineDownAct = new QAction(tr("Line down"), this);
    lineDownAct->setShortcut(tr("Ctrl+Shift+Down"));
    lineDownAct->setStatusTip(tr("Move current line downwards"));

    lineFillAct = new QAction(tr("Fill line"), this);
    lineFillAct->setShortcut(tr("Ctrl+Shift+D"));
    lineFillAct->setStatusTip(tr("Fill current line with selected substring"));

    linesJoinAct = new QAction(tr("Join lines"), this);
    linesJoinAct->setShortcut(tr("Ctrl+J"));
    linesJoinAct->setStatusTip(tr("Join selected lines"));
    connect(linesJoinAct, SIGNAL(triggered()), this, SLOT(joinLines()));

    toggleCommentAct = new QAction(QIcon(":/images/edit-comment.png"), tr("Toggle comment"), this);
    toggleCommentAct->setShortcut(tr("Ctrl+/"));
    toggleCommentAct->setData(Qt::Key_Slash | Qt::ControlModifier);
    toggleCommentAct->setStatusTip(tr("Toggle selected lines commented/uncommented"));
    connect(toggleCommentAct, SIGNAL(triggered()), this, SLOT(sendActionKeys()));

    autoTagAct = new QAction(QIcon(":/images/autotag.png"), tr("Auto tag"), this);
    autoTagAct->setShortcut(Qt::Key_Return | Qt::ControlModifier);
    autoTagAct->setStatusTip(tr("Complete current tag/Wrap current selection"));
    connect(autoTagAct, SIGNAL(triggered()), this, SLOT(autoTag()));



    tabsToSpacesAct = new QAction(QIcon(":/images/tabs-to-spaces.png"), tr("Tabs to spaces"), this);
//    tabsToSpacesAct->setShortcut(tr("Ctrl+Shift+Space"));
    tabsToSpacesAct->setStatusTip(tr("Convert tabs to spaces using current tab width value"));

    spacesToTabsAct = new QAction(QIcon(":/images/spaces-to-tabs.png"), tr("Spaces to tabs"), this);
//    spacesToTabsAct->setShortcut(tr("Ctrl+Shift+Tab"));
    spacesToTabsAct->setStatusTip(tr("Convert spaces to tabs using current tab width value"));

    trimRightSpacesAct = new QAction(QIcon(":/images/trim-last-spaces.png"), tr("Trim trailing spaces"), this);
//    trimRightSpacesAct->setShortcut(tr("Ctrl+Alt+Space"));
    trimRightSpacesAct->setStatusTip(tr("Remove all the trailing spaces of lines"));

    trimSpacesAct = new QAction(QIcon(":/images/compress-spaces.png"), tr("Compress spaces"), this);
    trimSpacesAct->setStatusTip(tr("Remove all the extra spaces between words"));

    trimAllEmptyLinesAct = new QAction(QIcon(":/images/trim-empty-lines.png"), tr("Trim All"), this);
    trimAllEmptyLinesAct->setStatusTip(tr("Remove all the empty lines"));
    connect(trimAllEmptyLinesAct, SIGNAL(triggered()), this, SLOT(trimAllEmptyLines()));

    trimAboveEmptyLinesAct = new QAction(QIcon(":/images/trim-empty-lines-more.png"), tr("Trim if more"), this);
    trimAboveEmptyLinesAct->setStatusTip(tr("Only trim if more than given empty lines"));
    connect(trimAboveEmptyLinesAct, SIGNAL(triggered()), this, SLOT(trimAboveEmptyLines()));

    trimBelowEmptyLinesAct = new QAction(QIcon(":/images/trim-empty-lines-less.png"), tr("Trim if less"), this);
    trimBelowEmptyLinesAct->setStatusTip(tr("Only trim if not more than given empty lines"));
    connect(trimBelowEmptyLinesAct, SIGNAL(triggered()), this, SLOT(trimBelowEmptyLines()));

    wordFormatAct = new QAction(QIcon(":/images/format-words.png"), tr("Format words..."), this);
    wordFormatAct->setStatusTip(tr("Format words in the document or selection"));
    wordFormatAct->setShortcut(tr("Ctrl+Shift+J"));
    connect(wordFormatAct, SIGNAL(triggered()), this, SLOT(formatWords()));

    numberAct = new QAction(QIcon(":/images/format-numbers.png"), tr("Format number..."), this);
    numberAct->setStatusTip(tr("Format numeric value"));
    numberAct->setShortcut(tr("Ctrl+Shift+N"));
    connect(numberAct, SIGNAL(triggered()), this, SLOT(formatNumber()));



    winEolAct = new QAction(QIcon(":/images/win22.png"), tr("Windows"), this);
    winEolAct->setStatusTip(tr("Set Windows-style line ends (CR/LF)"));
    winEolAct->setCheckable(true);
    winEolAct->setData(QsciScintilla::EolWindows);

    unixEolAct = new QAction(QIcon(":/images/linux22.png"), tr("Unix"), this);
    unixEolAct->setStatusTip(tr("Set Unix-style line ends (LF)"));
    unixEolAct->setCheckable(true);
    unixEolAct->setData(QsciScintilla::EolUnix);

    macEolAct = new QAction(QIcon(":/images/mac22.png"), tr("Mac"), this);
    macEolAct->setStatusTip(tr("Set Mac-style line ends (CR)"));
    macEolAct->setCheckable(true);
    macEolAct->setData(QsciScintilla::EolMac);

    eolsGroup = new QActionGroup(this);
    eolsGroup->addAction(winEolAct);
    eolsGroup->addAction(unixEolAct);
    eolsGroup->addAction(macEolAct);
    eolsGroup->setExclusive(false);



    zoomInAct = new QAction(QIcon(":/images/zoom-in.png"), tr("Zoom in"), this);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setStatusTip(tr("Increase text size"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(QIcon(":/images/zoom-out.png"), tr("Zoom out"), this);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setStatusTip(tr("Decrease text size"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    zoomBaseAct = new QAction(QIcon(":/images/zoom-original.png"), tr("Zoom 1:1"), this);
    zoomBaseAct->setStatusTip(tr("Zoom to original scale"));
    connect(zoomBaseAct, SIGNAL(triggered()), this, SLOT(zoomBase()));

    whiteSpacesAct = new QAction(QIcon(":/images/view-spaces.png"), tr("Show whitespaces"), this);
    whiteSpacesAct->setStatusTip(tr("Toggle visibility of spaces and tabs"));
    whiteSpacesAct->setCheckable(true);
    connect(whiteSpacesAct, SIGNAL(toggled(bool)), this, SLOT(showSpaces(bool)));

    eolsAct = new QAction(QIcon(":/images/view-eols.png"), tr("Show line ends"), this);
    eolsAct->setStatusTip(tr("Toggle visibility of line ends"));
    eolsAct->setCheckable(true);

    indentGuidesAct = new QAction(QIcon(":/images/view-guides.png"), tr("Show indentation guides"), this);
    indentGuidesAct->setStatusTip(tr("Toggle visibility of indentation guides"));
    indentGuidesAct->setCheckable(true);

    wrapAct = new QAction(QIcon(":/images/view-wrap.png"), tr("Wrap text"), this);
    wrapAct->setStatusTip(tr("Wrap text longer than border mark"));
    wrapAct->setCheckable(true);
    connect(wrapAct, SIGNAL(toggled(bool)), this, SLOT(wrapText(bool)));

    foldAct = new QAction(QIcon(":/images/folding.png"), tr("Toggle folding"), this);
    foldAct->setStatusTip(tr("Toggle expanded state of the folded blocks"));

    foldExpandAct = new QAction(QIcon(":/images/folding-expand.png"), tr("Unfold all"), this);
    foldExpandAct->setStatusTip(tr("Expand all the folded blocks"));

    foldContractAct = new QAction(QIcon(":/images/folding-contract.png"), tr("Fold all"), this);
    foldContractAct->setStatusTip(tr("Fold all the text"));

    infoAct = new QAction(QIcon(":/images/view-info.png"), tr("Document info"), this);
    infoAct->setShortcut(tr("Ctrl+I"));
    infoAct->setStatusTip(tr("Show summary info about the document"));
    connect(infoAct, SIGNAL(triggered()), this, SLOT(documentInfo()));



    addBookmarkAct = new QAction(QIcon(":/images/ledgreen.png"), tr("Toggle bookmark"), this);
    addBookmarkAct->setStatusTip(tr("Set/remove a bookmark for the current line"));
#ifdef Q_WS_X11
    addBookmarkAct->setShortcut(tr("Ctrl+Shift+F2"));
#else
    addBookmarkAct->setShortcut(tr("Ctrl+F2"));
#endif
    addBookmarkAct->setData(-1);
    connect(addBookmarkAct, SIGNAL(triggered()), this, SLOT(toggleBookmarks()));

    removeAllBookmarksAct = new QAction(tr("Remove all bookmarks"), this);
    removeAllBookmarksAct->setStatusTip(tr("Remove all bookmarks in the document"));
    removeAllBookmarksAct->setData(-1);
    connect(removeAllBookmarksAct, SIGNAL(triggered()), this, SLOT(removeBookmarks()));

    nextBookmarkAct = new QAction(QIcon(":/images/bookmark-next.png"), tr("Next bookmark"), this);
    nextBookmarkAct->setStatusTip(tr("Go to the next bookmark"));
    nextBookmarkAct->setShortcut(tr("F2"));
    nextBookmarkAct->setData(-1);

    prevBookmarkAct = new QAction(QIcon(":/images/bookmark-prev.png"), tr("Previous bookmark"), this);
    prevBookmarkAct->setStatusTip(tr("Go to the previous bookmark"));
    prevBookmarkAct->setShortcut(tr("Shift+F2"));
    prevBookmarkAct->setData(-1);



    searchAct = new QAction(QIcon(":/images/search.png"), tr("Search..."), this);
    searchAct->setStatusTip(tr("Search for a substring in the document"));
    searchAct->setShortcut(QKeySequence::Find);
    connect(searchAct, SIGNAL(triggered()), this, SLOT(search()));

    searchFilesAct = new QAction(tr("Search in open files..."), this);
    searchFilesAct->setStatusTip(tr("Search for a substring in the open documents"));
    searchFilesAct->setShortcut(tr("Shift+Ctrl+F"));
    connect(searchFilesAct, SIGNAL(triggered()), this, SLOT(searchFiles()));

    searchNextAct = new QAction(QIcon(":/images/search-next.png"), tr("Find next"), this);
    searchNextAct->setStatusTip(tr("Search for the next entry of the substring in the document"));
    searchNextAct->setShortcut(QKeySequence::FindNext);
    connect(searchNextAct, SIGNAL(triggered()), this, SLOT(searchNext()));

    searchPrevAct = new QAction(QIcon(":/images/search-prev.png"), tr("Find previous"), this);
    searchPrevAct->setStatusTip(tr("Search for the previous entry of the substring in the document"));
    searchPrevAct->setShortcut(QKeySequence::FindPrevious);
    connect(searchPrevAct, SIGNAL(triggered()), this, SLOT(searchPrev()));

    searchBraceAct = new QAction(QIcon(":/images/search-braces.png"), tr("Find matching brace"), this);
    searchBraceAct->setStatusTip(tr("Search for the next or previous matching brace"));
    searchBraceAct->setShortcut(tr("Ctrl+E"));

    selectBraceAct = new QAction(QIcon(":/images/select-braces.png"), tr("Select to matching brace"), this);
    selectBraceAct->setStatusTip(tr("Select text between the current and next/previous matching brace"));
    selectBraceAct->setShortcut(tr("Ctrl+Shift+E"));



    navigateAct = new QAction(tr("Go to line..."), this);
    navigateAct->setStatusTip(tr("Quick jump to particular line"));
    navigateAct->setShortcut(tr("Ctrl+G"));
    connect(navigateAct, SIGNAL(triggered()), this, SLOT(navigate()));



    closeTabAct = new QAction(QIcon(":/images/tab-close.png"), tr("Close current"), this);
    closeTabAct->setShortcut(tr("Ctrl+W"));
    closeTabAct->setStatusTip(tr("Close current window"));
    connect(closeTabAct, SIGNAL(triggered()), this, SLOT(closeTab()));

    closeAllExceptAct = new QAction(tr("Close all except current"), this);
    closeAllExceptAct->setStatusTip(tr("Close all windows except current"));
    connect(closeAllExceptAct, SIGNAL(triggered()), this, SLOT(closeTabExcept()));

    closeAllAct = new QAction(tr("Close all"), this);
    closeAllAct->setStatusTip(tr("Close all opened windows"));
    connect(closeAllAct, SIGNAL(triggered()), this, SLOT(closeAllWindows()));

    copyPathAct = new QAction(tr("Copy full path"), this);
    copyPathAct->setStatusTip(tr("Copy path to the current file into clipboard"));
    connect(copyPathAct, SIGNAL(triggered()), this, SLOT(copyTabPath()));

    copyNameAct = new QAction(tr("Copy only name"), this);
    copyNameAct->setStatusTip(tr("Copy name of the current file into clipboard"));
    connect(copyNameAct, SIGNAL(triggered()), this, SLOT(copyTabName()));



    explorerFileAct = new QAction(QIcon(":/images/view-sidetree.png"), tr("Show in File Explorer"), this);
    explorerFileAct->setShortcut(QKeySequence("Alt+Ctrl+0"));
    //explorerFileAct->setShortcutContext(Qt::ApplicationShortcut);
    explorerFileAct->setStatusTip(tr("Show current file in the File Explorer"));
    connect(explorerFileAct, SIGNAL(triggered()), this, SLOT(exploreFile()));

    configAct = new QAction(QIcon(":/images/configure.png"), tr("Preferences..."), this);
    //configAct->setShortcut(QKeySequence::Preferences);
    configAct->setStatusTip(tr("Change current preferences"));
    connect(configAct, SIGNAL(triggered()), this, SLOT(showConfig()));

    toolsAct = new QAction(tr("External tools..."), this);
    toolsAct->setStatusTip(tr("Show Tool Manager"));
    connect(toolsAct, SIGNAL(triggered()), toolManagerDialog, SLOT(exec()));



    saveSessionAct = new QAction(QIcon(":/images/project-save.png"), tr("Store"), this);
    saveSessionAct->setStatusTip(tr("Store open windows as the current session"));
    connect(saveSessionAct, SIGNAL(triggered()), this, SLOT(saveSession()));

    saveSessionAsAct = new QAction(tr("Store as..."), this);
    saveSessionAsAct->setStatusTip(tr("Store open windows as a new session"));
    connect(saveSessionAsAct, SIGNAL(triggered()), this, SLOT(saveSessionAs()));

    reloadSessionAct = new QAction(QIcon(":/images/project-reload.png"), tr("Reload"), this);
    reloadSessionAct->setStatusTip(tr("Reload current session"));
    connect(reloadSessionAct, SIGNAL(triggered()), this, SLOT(reloadSession()));

    removeSessionAct = new QAction(QIcon(":/images/remove.png"), tr("Remove"), this);
    removeSessionAct->setStatusTip(tr("Remove current session"));
    connect(removeSessionAct, SIGNAL(triggered()), this, SLOT(removeSession()));

    closeSessionAct = new QAction(QIcon(":/images/dialog-close.png"), tr("Close"), this);
    closeSessionAct->setStatusTip(tr("Close current session"));
    connect(closeSessionAct, SIGNAL(triggered()), this, SLOT(closeSession()));



    aboutAct = new QAction(tr("&About..."), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    shortcutsAct = new QAction(tr("Show shortcuts..."), this);
    shortcutsAct->setShortcut(QKeySequence::HelpContents);
    shortcutsAct->setStatusTip(tr("Show active shortcuts list"));
    connect(shortcutsAct, SIGNAL(triggered()), this, SLOT(showShortcuts()));


    QMenu *hMenu;
    hMenu = createHighlightMenu(QIcon(":/images/ledlightblue.png"),
                        tr("Mark style 1"), INDICATOR_HIGHLIGHT1);
    highlightMenuList.append(hMenu);
    hMenu = createHighlightMenu(QIcon(":/images/ledlightgreen.png"),
                        tr("Mark style 2"), INDICATOR_HIGHLIGHT2);
    highlightMenuList.append(hMenu);
    hMenu = createHighlightMenu(QIcon(":/images/ledpurple.png"),
                        tr("Mark style 3"), INDICATOR_HIGHLIGHT3);
    highlightMenuList.append(hMenu);
    hMenu = createHighlightMenu(QIcon(":/images/ledred.png"),
                        tr("Mark style 4"), INDICATOR_HIGHLIGHT4);
    highlightMenuList.append(hMenu);
    hMenu = createHighlightMenu(QIcon(":/images/ledblue.png"),
                        tr("Mark style 5"), INDICATOR_HIGHLIGHT5);
    highlightMenuList.append(hMenu);
    hMenu = createHighlightMenu(QIcon(":/images/ledorange.png"),
                        tr("Mark style 6"), INDICATOR_HIGHLIGHT6);
    highlightMenuList.append(hMenu);

    clearAllHlAct = new QAction(QIcon(":/images/empty.png"), tr("Clear all markup"), this);
    connect(clearAllHlAct, SIGNAL(triggered()), this, SLOT(highlightClearAll()));



    previewAct = new QAction(QIcon(":/images/view-preview.png"), tr("Preview..."), this);
    previewAct->setStatusTip(tr("Preview current document as final output (if possible)"));
    previewAct->setShortcut(tr("F12"));
    connect(previewAct, SIGNAL(triggered()), this, SLOT(documentPreview()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(reloadAct);
    fileMenu->addMenu(codecOpenMenu);
    fileMenu->addMenu(recentFiles->menu());
    fileMenu->addSeparator();
    fileMenu->addMenu(tabListMenu);
    fileMenu->addAction(closeTabAct);
    fileMenu->addAction(closeAllExceptAct);
    fileMenu->addAction(closeAllAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAct);
    fileMenu->addMenu(codecSaveMenu);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(saveAllAct);
    fileMenu->addSeparator();
    fileMenu->addAction(bomAct);
    fileMenu->addAction(readOnlyAct);
    fileMenu->addSeparator();
    fileMenu->addAction(infoAct);
    fileMenu->addSeparator();
    fileMenu->addAction(copyPathAct);
    fileMenu->addAction(copyNameAct);
    fileMenu->addSeparator();
    fileMenu->addAction(explorerAct);
    fileMenu->addAction(explorerFileAct);
    fileMenu->addSeparator();
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);



    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addSeparator();
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);

    editMenu->addSeparator();
    editMenu->addAction(upperAct);
    editMenu->addAction(lowerAct);
    editMenu->addAction(lineSwapAct);
    editMenu->addAction(lineUpAct);
    editMenu->addAction(lineDownAct);
    editMenu->addAction(lineDuplicateAct);
    editMenu->addAction(lineFillAct);
    editMenu->addAction(linesJoinAct);
    editMenu->addAction(lineKillAct);
    editMenu->addAction(lineStartKillAct);
    editMenu->addAction(lineEndKillAct);
    editMenu->addAction(wordKillAct);
    editMenu->addAction(wordStartKillAct);
    editMenu->addAction(wordEndKillAct);



    formatMenu = menuBar()->addMenu(tr("Forma&t"));
    editEolsMenu = formatMenu->addMenu(QIcon(":/images/view-eols.png"), tr("Line ends"));
    editEolsMenu->addAction(winEolAct);
    editEolsMenu->addAction(unixEolAct);
    editEolsMenu->addAction(macEolAct);

    formatMenu->addSeparator();
    formatMenu->addAction(tabsToSpacesAct);
    formatMenu->addAction(spacesToTabsAct);
    formatMenu->addAction(trimRightSpacesAct);
    formatMenu->addAction(trimSpacesAct);
    formatMenu->addSeparator();
    formatMenu->addAction(toggleCommentAct);
    formatMenu->addAction(autoTagAct);
    formatMenu->addAction(wordFormatAct);
    formatMenu->addAction(numberAct);
    formatMenu->addSeparator();

    // add formatters
    if (formatManager->available(FormatManager::FT_ASTYLE)) {
      QMenu *formatCodeMenu = formatManager->menu(FormatManager::FT_ASTYLE);
      formatMenu->addMenu(formatCodeMenu);
      connect(formatCodeMenu, SIGNAL(triggered(QAction*)), this, SLOT(formatCode(QAction*)));
    }
    if (formatManager->available(FormatManager::FT_UNCRUSTIFY)) {
      QMenu *formatCodeMenu = formatManager->menu(FormatManager::FT_UNCRUSTIFY);
      formatMenu->addMenu(formatCodeMenu);
      connect(formatCodeMenu, SIGNAL(triggered(QAction*)), this, SLOT(formatCode(QAction*)));
    }
    if (formatManager->available(FormatManager::FT_CSSTIDY)) {
      QMenu *formatCodeMenu = formatManager->menu(FormatManager::FT_CSSTIDY);
      formatMenu->addMenu(formatCodeMenu);
      connect(formatCodeMenu, SIGNAL(triggered(QAction*)), this, SLOT(formatCode(QAction*)));
    }
    if (formatManager->available(FormatManager::FT_HTMLTIDY)) {
      QMenu *formatCodeMenu = formatManager->menu(FormatManager::FT_HTMLTIDY);
      formatMenu->addMenu(formatCodeMenu);
      connect(formatCodeMenu, SIGNAL(triggered(QAction*)), this, SLOT(formatCode(QAction*)));
    }



    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(zoomBaseAct);
    viewMenu->addSeparator();
    viewMenu->addAction(indentGuidesAct);
    viewMenu->addAction(whiteSpacesAct);
    viewMenu->addAction(eolsAct);
    viewMenu->addSeparator();
    viewMenu->addAction(wrapAct);
    viewMenu->addSeparator();
    viewMenu->addAction(foldContractAct);
    viewMenu->addAction(foldExpandAct);
    viewMenu->addAction(foldAct);
    viewMenu->addSeparator();
    viewMenu->addAction(previewAct);



    searchMenu = menuBar()->addMenu(tr("&Search"));
    searchMenu->addAction(searchAct);
    searchMenu->addAction(searchNextAct);
    searchMenu->addAction(searchPrevAct);
    searchMenu->addAction(searchFilesAct);
    searchMenu->addSeparator();
    searchMenu->addAction(searchBraceAct);
    searchMenu->addAction(selectBraceAct);
    searchMenu->addSeparator();
    searchMenu->addAction(navigateAct);



    markupMenu = menuBar()->addMenu(tr("&Markup"));
    foreach(QMenu *menu, highlightMenuList)
      markupMenu->addMenu(menu);

    markupMenu->addSeparator();
    markupMenu->addAction(clearAllHlAct);



    stylesMenu = menuBar()->addMenu(tr("S&yntax"));
    styleManager->setMenu(stylesMenu);



    bookmarkMenu = menuBar()->addMenu(tr("&Bookmarks"));
    connect(bookmarkMenu, SIGNAL(aboutToShow()), this, SLOT(fillBookmarkMenu()));
    connect(bookmarkMenu, SIGNAL(triggered(QAction*)), this, SLOT(bookmarkActivated(QAction*)));
    bookmarkMenu->addAction(addBookmarkAct);
    bookmarkMenu->addAction(removeAllBookmarksAct);
    bookmarkMenu->addAction(nextBookmarkAct);
    bookmarkMenu->addAction(prevBookmarkAct);



    sessionMenu = menuBar()->addMenu(tr("Sess&ions"));
    sessionListMenu = sessionMenu->addMenu(QIcon(":/images/tab-list.png"), tr("Available sessions"));
    connect(sessionListMenu, SIGNAL(triggered(QAction*)), this, SLOT(sessionActivated(QAction*)));
    sessionMenu->addSeparator();
    sessionMenu->addAction(closeSessionAct);
    sessionMenu->addSeparator();
    sessionMenu->addAction(reloadSessionAct);
    sessionMenu->addAction(saveSessionAct);
    sessionMenu->addAction(saveSessionAsAct);
    sessionMenu->addSeparator();
    sessionMenu->addAction(removeSessionAct);


    toolsMenu = menuBar()->addMenu(tr("Too&ls"));
    toolsMenu->addAction(toolsAct);
    toolsMenu->addAction(configAct);
    toolsMenu->addSeparator();
    toolBarsMenu = toolsMenu->addMenu(tr("Toolbars"));
    toolsMenu->addAction(symbolsAct);
    toolsMenu->addAction(explorerAct);


    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(shortcutsAct);



    // create tab menu
    tabMenu = new QMenu(this);
    tabMenu->addAction(closeTabAct);
    tabMenu->addAction(closeAllExceptAct);
    tabMenu->addAction(closeAllAct);
    tabMenu->addSeparator();
    tabMenu->addAction(reloadAct);
    tabMenu->addMenu(codecOpenMenu);
    tabMenu->addSeparator();
    tabMenu->addAction(saveAct);
    tabMenu->addAction(saveAsAct);
    tabMenu->addMenu(codecSaveMenu);
    tabMenu->addSeparator();
    tabMenu->addAction(bomAct);
    tabMenu->addAction(readOnlyAct);
    tabMenu->addSeparator();
    tabMenu->addAction(copyPathAct);
    tabMenu->addAction(copyNameAct);
    tabMenu->addSeparator();
    tabMenu->addAction(explorerFileAct);


    // create context menu
    editContextMenu = new QMenu(this);
    editContextMenu->addAction(undoAct);
    editContextMenu->addAction(redoAct);
    editContextMenu->addSeparator();
    editContextMenu->addAction(copyAct);
    editContextMenu->addAction(cutAct);
    editContextMenu->addAction(pasteAct);
    editContextMenu->addSeparator();
    foreach (QMenu *m, highlightMenuList)
      editContextMenu->addMenu(m);
    editContextMenu->addAction(clearAllHlAct);
    editContextMenu->addSeparator();
    editContextMenu->addAction(foldContractAct);
    editContextMenu->addAction(foldExpandAct);
    editContextMenu->addAction(foldAct);
    editContextMenu->addSeparator();
    editContextMenu->addSeparator();
    editContextMenu->addAction(toggleCommentAct);
    editContextMenu->addAction(autoTagAct);
}

QMenu* MainWindow::createHighlightMenu(const QIcon &icon, const QString &title, int id)
{
  QAction *act;
  QMenu *menu = new QMenu(title, this);
  menu->setIcon(icon);
  act = menu->addAction(tr("Selection only"), this, SLOT(highlightSelection()));
  act->setData(id);
  connect(menu->menuAction(), SIGNAL(triggered()), act, SIGNAL(triggered()));
  act = menu->addAction(tr("Similar to selection"), this, SLOT(highlightSimilarToSelection()));
  act->setData(id);
  menu->addSeparator();
  act = menu->addAction(tr("Clear all"), this, SLOT(highlightClear()));
  act->setData(id);
  act = menu->addAction(tr("Clear in selection"), this, SLOT(highlightClearInSelection()));
  act->setData(id);
  return menu;
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName("fileToolBar");
    fileToolBar->setIconSize(QSize(16,16));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(reloadAct);
    fileToolBar->addSeparator();
    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(saveAsAct);
    fileToolBar->addAction(saveAllAct);
    fileToolBar->addSeparator();
    fileToolBar->addAction(printAct);
    fileToolBar->addSeparator();
    fileToolBar->addAction(configAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName("editToolBar");
    editToolBar->setIconSize(QSize(16,16));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addSeparator();
    editToolBar->addAction(undoAct);
    editToolBar->addAction(redoAct);

    searchToolBar = addToolBar(tr("Search"));
    searchToolBar->setObjectName("searchToolBar");
    searchToolBar->setIconSize(QSize(16,16));
    searchToolBar->addAction(searchAct);
    searchToolBar->addAction(searchPrevAct);
    searchToolBar->addAction(searchNextAct);
    searchToolBar->addSeparator();
    searchToolBar->addAction(addBookmarkAct);
    searchToolBar->addAction(prevBookmarkAct);
    searchToolBar->addAction(nextBookmarkAct);

    formatToolBar = addToolBar(tr("Format"));
    formatToolBar->setObjectName("formatToolBar");
    formatToolBar->setIconSize(QSize(16,16));
    formatToolBar->addAction(toggleCommentAct);
    formatToolBar->addAction(autoTagAct);
    formatToolBar->addAction(wordFormatAct);
    formatToolBar->addAction(numberAct);
    formatToolBar->addSeparator();
    formatToolBar->addAction(tabsToSpacesAct);
    formatToolBar->addAction(spacesToTabsAct);
    formatToolBar->addAction(trimSpacesAct);
    formatToolBar->addAction(trimRightSpacesAct);

    viewToolBar = addToolBar(tr("View"));
    viewToolBar->setObjectName("viewToolBar");
    viewToolBar->setIconSize(QSize(16,16));
    viewToolBar->addAction(zoomInAct);
    viewToolBar->addAction(zoomOutAct);
    viewToolBar->addAction(zoomBaseAct);
    viewToolBar->addSeparator();
    viewToolBar->addAction(indentGuidesAct);
    viewToolBar->addAction(whiteSpacesAct);
    viewToolBar->addAction(eolsAct);
    viewToolBar->addSeparator();
    viewToolBar->addAction(wrapAct);
    viewToolBar->addSeparator();
    viewToolBar->addAction(foldContractAct);
    viewToolBar->addAction(foldExpandAct);
    viewToolBar->addAction(foldAct);
    viewToolBar->addSeparator();
    viewToolBar->addAction(previewAct);


    markupToolBar = addToolBar(tr("Markup"));
    markupToolBar->setObjectName("markupToolBar");
    markupToolBar->setIconSize(QSize(16,16));

    foreach(QMenu *menu, highlightMenuList)
      markupToolBar->addAction(menu->menuAction());
    markupToolBar->addSeparator();
    markupToolBar->addAction(clearAllHlAct);


    createTrimBar();


    // toolbars menu
    toolBarsMenu->addAction(fileToolBar->toggleViewAction());
    toolBarsMenu->addAction(editToolBar->toggleViewAction());
    toolBarsMenu->addAction(formatToolBar->toggleViewAction());
    toolBarsMenu->addAction(trimToolBar->toggleViewAction());
    toolBarsMenu->addAction(viewToolBar->toggleViewAction());
    toolBarsMenu->addAction(searchToolBar->toggleViewAction());
    toolBarsMenu->addAction(markupToolBar->toggleViewAction());
}

QLayout* MainWindow::addStatusBarSection()
{
  QWidget *section = new QWidget(statusBar());
  QHBoxLayout *hbl2 = new QHBoxLayout();
  hbl2->setSpacing(1);
  hbl2->setMargin(1);
  section->setLayout(hbl2);
  statusBar()->addPermanentWidget(section);
  return hbl2;
}

void MainWindow::createStatusBar()
{
  QStatusBar *sb = statusBar();
  sb->showMessage(tr("Ready"));

  addStatusBarSection()->addWidget(codecLabel = new ClickLabel(sb));
  codecLabel->setMinimumWidth(120);
  QMenu *codecMenu = new QMenu(this);
  codecMenu->addMenu(codecOpenMenu);
  codecMenu->addMenu(codecSaveMenu);
  codecMenu->addSeparator();
  codecMenu->addAction(bomAct);
  codecLabel->setMenu(codecMenu);

  addStatusBarSection()->addWidget(eolLabel = new ClickLabel(sb));
  //eolLabel->setMinimumWidth(40);
  eolLabel->setMenu(editEolsMenu);

  QLayout *l = addStatusBarSection();
  l->addWidget(rowLabel = new ClickLabel(sb));
  rowLabel->setMinimumWidth(60);
  connect(rowLabel, SIGNAL(clicked()), this, SLOT(navigate()));
  l->addWidget(columnLabel = new ClickLabel(sb));
  columnLabel->setMinimumWidth(60);
  connect(columnLabel, SIGNAL(clicked()), this, SLOT(navigate()));


  l = addStatusBarSection();
  l->addWidget(bytesLabel = new ClickLabel(sb));
  bytesLabel->setMinimumWidth(80);
  connect(bytesLabel, SIGNAL(clicked()), this, SLOT(navigate()));
//  hbl1->addWidget(charsLabel = new QLabel(sb));
//  charsLabel->setMinimumWidth(80);
  l->addWidget(linesLabel = new ClickLabel(sb));
  linesLabel->setMinimumWidth(80);
  connect(linesLabel, SIGNAL(clicked()), this, SLOT(navigate()));
}

void MainWindow::createTabBar()
{
  tabWidget = new TabWidget(mainWidget);
  mainWidget->layout()->addWidget(tabWidget);

  tabWidget->setTabsClosable(true);
  tabWidget->setMovable(true);
  //tabWidget->setUsesScrollButtons(false);
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
  connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
  tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(tabWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTabMenu(const QPoint&)));

  tabListMenu = new QMenu(tr("Opened files"), mainWidget);
  tabListMenu->setIcon(QIcon(":/images/windows-list.png"));
  connect(tabListMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowsMenu()));
  connect(tabListMenu, SIGNAL(triggered(QAction*)), this, SLOT(switchToWindow(QAction*)));

  tabListButton = new QToolButton(mainWidget);
  tabListButton->setIcon(tabListMenu->icon());
  tabListButton->setMenu(tabListMenu);
  tabListButton->setPopupMode(QToolButton::InstantPopup);
  tabWidget->setCornerWidget(tabListButton);
}

void MainWindow::fillWindowsMenu()
{
  tabListMenu->clear();

  for (int i = 0; i < tabWidget->count(); i++)
  {
    EditWindow *ew = (EditWindow*) tabWidget->widget(i);
    QAction *act = tabListMenu->addAction(ew->fullName());
    act->setData(i);
    if (tabWidget->currentIndex() == i)
    {
      QFont f(act->font());
      f.setBold(true);
      //act->setDisabled(true);
      act->setFont(f);
      act->setCheckable(true);
      act->setChecked(true);
    }
  }
}

void MainWindow::switchToWindow(QAction *act)
{
  int idx = act->data().toInt();
  tabWidget->setCurrentIndex(idx);
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::closeAllWindows()
{
  if (maybeSave())
  {
    tabWidget->blockSignals(true);

    while (tabWidget->count())
    {
      textEdit = (EditWindow*) tabWidget->widget(0);
      disconnectSignals();
      tabWidget->removeTab(0);
      delete textEdit;
    }

    tabWidget->blockSignals(false);

    textEdit = 0;
    newFile();
  }
}

void MainWindow::closeTabExcept()
{
  int idx = tabWidget->currentIndex();
  if (maybeSave(idx))
  {
    tabWidget->blockSignals(true);

    for (int i = tabWidget->count()-1; i >= 0; i--)
    {
      textEdit = (EditWindow*) tabWidget->widget(i);
      disconnectSignals();

      if (idx == i)
        continue;

      tabWidget->removeTab(i);
      delete textEdit;
    }

    tabWidget->blockSignals(false);

    // reconnect
    textEdit = (EditWindow*) tabWidget->widget(0);
    connectSignals();
  }
}

void MainWindow::closeTab()
{
  tabCloseRequested(tabWidget->currentIndex());
}

void MainWindow::copyTabPath()
{
  QApplication::clipboard()->setText(textEdit->fileName());
}

void MainWindow::copyTabName()
{
  QApplication::clipboard()->setText(textEdit->name());
}

void MainWindow::newFile()
{
  createTab("");
}

void MainWindow::createTab(const QString &fileName)
{
  disconnectSignals();

  textEdit = new EditWindow(this, fileName);
  textEdit->applyConfig(config);
  textEdit->setEolMode((QsciScintilla::EolMode)config->defaultEOL);
  textEdit->setCodec(config->defaultCodec);
  textEdit->setBOM(config->defaultBOM);
  textEdit->setModified(false);

  int idx = tabWidget->addTab(textEdit, textEdit->name());
  tabWidget->setCurrentWidget(textEdit);
  tabWidget->setTabToolTip(idx, textEdit->fullName());
  tabWidget->setTabIcon(idx, QIcon(":/images/file_saved.png"));

  // default style
  const EditStyle &defStyle = styleManager->editStyle();

  // define search indicator
  textEdit->createIndicator(INDICATOR_SEARCH,
                         INDIC_ROUNDBOX,
                         defStyle.SearchIndicatorColor
                         );

  // define quick search indicators
  textEdit->createIndicator(INDICATOR_QUICK_SEARCH1,
                         INDIC_ROUNDBOX,
                         defStyle.HighlightIndicator1Color
                         );

  textEdit->createIndicator(INDICATOR_QUICK_SEARCH2,
                         INDIC_ROUNDBOX,
                         defStyle.HighlightIndicator2Color
                         );

  // define highlight indicators
  textEdit->createIndicator(INDICATOR_HIGHLIGHT1, INDIC_ROUNDBOX, Qt::cyan);
  textEdit->createIndicator(INDICATOR_HIGHLIGHT2, INDIC_ROUNDBOX, Qt::green);
  textEdit->createIndicator(INDICATOR_HIGHLIGHT3, INDIC_ROUNDBOX, Qt::magenta);
  textEdit->createIndicator(INDICATOR_HIGHLIGHT4, INDIC_ROUNDBOX, Qt::red);
  textEdit->createIndicator(INDICATOR_HIGHLIGHT5, INDIC_ROUNDBOX, Qt::blue);
  //textEdit->createIndicator(INDICATOR_HIGHLIGHT6, INDIC_ROUNDBOX, Qt::yellow);
  textEdit->createIndicator(INDICATOR_HIGHLIGHT6, INDIC_ROUNDBOX, QColor(0xff9900));

  // event filter
  textEdit->installEventFilter(this);
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::tabCloseRequested(int idx)
{
  EditWindow* te = (EditWindow*) tabWidget->widget(idx);

  if (te->isModified())
  {
    int ret = QMessageBox::warning(this, tr("STE"),
                 tr("The document %1 has been modified.\n"
                    "Do you want to save your changes?").arg(te->fileName()),
                 QMessageBox::Yes |
                 QMessageBox::No |
                 QMessageBox::Cancel);

    if (ret == QMessageBox::Cancel)
      return;

    if (ret == QMessageBox::Yes)
    {
      tabWidget->setCurrentIndex(idx);
      if (!save())
        return;
    }
  }

  if (tabWidget->count() == 1)
  {
    newFile();
  }

  tabWidget->removeTab(idx);
  delete te;
}

void MainWindow::showTabMenu(const QPoint& p)
{
  int idx = tabWidget->tabBar()->tabAt(p);
  if (idx < 0)
    return;
  tabWidget->setCurrentIndex(idx);
  tabMenu->popup(tabWidget->mapToGlobal(p));
}

void MainWindow::currentTabChanged(int idx)
{
  disconnectSignals();
  textEdit = (EditWindow*) tabWidget->currentWidget();
  if (!textEdit)
      return;
  connectSignals();

  updateFileSavedState(textEdit, idx);

  textEdit->zoomTo(zoom);

  textEdit->setWhitespaceVisibility(
      whiteSpacesAct->isChecked() ? EditWindow::WsVisible : EditWindow::WsInvisible);
  textEdit->setEolVisibility(eolsAct->isChecked());
  textEdit->setIndentationGuides(indentGuidesAct->isChecked());
  textEdit->setWrapMode(wrapAct->isChecked() ? EditWindow::WrapWord : EditWindow::WrapNone);

  textEdit->setFocus();

  updateCurrentFileStatus();

  updateBookmarks();

  // update file explorer if needed
  explorerBar->synchronizeIfNeeded(textEdit->fullName());
}

void MainWindow::updateFileTab(EditWindow *edit, int idx)
{
  tabWidget->setTabToolTip(idx,
                           QString("%1\n%2")
                           .arg(edit->fullName())
                           .arg(edit->lastModified().toString(Qt::SystemLocaleShortDate))
                           );

  if (tabWidget->tabText(idx) != edit->name())
    tabWidget->setTabText(idx, edit->name());

  if (edit->fileName().length())
    lastFileDir = QFileInfo(edit->fileName()).absolutePath();
}

void MainWindow::updateFileSavedState(EditWindow *edit, int idx)
{
  if (idx == tabWidget->currentIndex())
  {
    setWindowTitle(tr("%1[*] - %2").arg(edit->fullName(), "STE"));
    setWindowModified(edit->isModified());
  }

  updateFileTab(edit, idx);

  updateFileTabIcon(edit, idx);

  reloadAct->setEnabled(!edit->justCreated());
  codecOpenMenu->setEnabled(!edit->justCreated());
  explorerFileAct->setEnabled(!edit->justCreated());
  readOnlyAct->setChecked(edit->isReadOnly());
}

void MainWindow::updateFileTabIcon(EditWindow *edit, int idx)
{
  if (edit->isReadOnly())
    tabWidget->setTabIcon(idx, edit->isModified() ?
                          QIcon(":/images/file_unsaved_ro.png") :
                          QIcon(":/images/file_saved_ro.png"));
  else
    tabWidget->setTabIcon(idx, edit->isModified() ?
                          QIcon(":/images/file_unsaved.png") :
                          QIcon(":/images/file_saved.png"));
}

void MainWindow::updateCurrentFileStatus()
{
  styleManager->setCurrentLexer(textEdit->lexerType());

  eolsChanged();
  codecChanged();

  documentChanged();
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::documentWasModified(bool /*m*/)
{
  updateFileSavedState(textEdit, tabWidget->currentIndex());

  documentChanged();
}

void MainWindow::documentChanged()
{
  linesChanged();

  int line, index;
  textEdit->getCursorPosition(&line, &index);
  cursorPositionChanged(line, index);

  bytesLabel->setText(tr("Symbols: %1  ").arg(textEdit->length()));
//  charsLabel->setText(tr("Chars: %1").arg(textEdit->text().length()));

  undoAct->setEnabled(textEdit->isUndoAvailable());
  redoAct->setEnabled(textEdit->isRedoAvailable());

  cutAct->setEnabled(textEdit->hasSelectedText());
  copyAct->setEnabled(textEdit->hasSelectedText());
}

void MainWindow::linesChanged()
{
  linesLabel->setText(tr("Lines: %1  ").arg(textEdit->lines()));
}

void MainWindow::cursorPositionChanged(int line, int pos)
{
  rowLabel->setText(tr("Line: %1  ").arg(line+1));
  columnLabel->setText(tr("Col: %1").arg(pos+1));
}

void MainWindow::eolsChanged()
{
  switch (textEdit->eolMode())
  {
  case QsciScintilla::EolWindows:
    eolLabel->setStatusTip("Line ends: Windows-style (CR/LF)");
    eolLabel->setPixmap(QPixmap(":/images/win22.png"));
    winEolAct->setChecked(true);
    break;
  case QsciScintilla::EolUnix :
    eolLabel->setStatusTip("Line ends: Unix-style (LF)");
    eolLabel->setPixmap(QPixmap(":/images/linux22.png"));
    unixEolAct->setChecked(true);
    break;
  default:
    eolLabel->setStatusTip("Line ends: Mac-style (CR)");
    eolLabel->setPixmap(QPixmap(":/images/mac22.png"));
    macEolAct->setChecked(true);
    break;
  }
  winEolAct->setChecked(textEdit->eolMode() == QsciScintilla::EolWindows);
  unixEolAct->setChecked(textEdit->eolMode() == QsciScintilla::EolUnix);
  macEolAct->setChecked(textEdit->eolMode() == QsciScintilla::EolMac);
}

void MainWindow::codecChanged()
{
  QString codec = textEdit->codec();
  if (codec.startsWith("UTF", Qt::CaseInsensitive))
  {
    codec += textEdit->bom() ? " [BOM]" : " [no BOM]";
  }

  codecLabel->setText(codec);
  codecLabel->setStatusTip(tr("Document encoding: ") + codec);

  bomAct->setChecked(textEdit->bom());
}

void MainWindow::bomChanged(bool bom)
{
  textEdit->setBOM(bom);
  codecChanged();
}

void MainWindow::marginClicked(int /*margin*/, int line, Qt::KeyboardModifiers)
{
  textEdit->toggleBookmark(line);
  updateBookmarks();
}

void MainWindow::lexerChanged(int lt)
{
  QsciLexer *lexer = styleManager->createLexer(lt);
  textEdit->setLexer(lexer, lt);
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::about()
{
  QString edition;
#ifdef Q_WS_WIN
  edition = "MS Windows edition";
#elif defined Q_WS_X11
  edition = "X11 edition";
#elif defined Q_WS_MAC
  edition = "MacOS edition";
#elif defined Q_WS_OS2
  edition = "OS/2 edition";
#endif

  QMessageBox aboutBox(this);
  aboutBox.setIconPixmap(QPixmap(":/images/ste.png"));
  aboutBox.setWindowTitle(tr("About STE"));
  aboutBox.setInformativeText(tr("<b>Sintegrial Text Editor</b><br>"
               "version %1<br>"
               "<br>"
               "%2"
               "<hr>"
               "freeware&nbsp;plain&nbsp;text&nbsp;editor&nbsp;by&nbsp;<a href='www.sintegrial.com'>Sintegrial&nbsp;Technologies</a><br>"
               "for Windows, Unix, Linux and other platforms<br>"
               "<br>"
               "based on Qt %3 and Qscintilla %4"
               "<br>"
               "uses iconv, uncrustify, enca"
               "<br>"
               "supports Artistic Style, CSS Tidy, HTML Tidy"
               "<br>"
               "uses free graphics from OpenDesktop project")
            .arg(STE_VERSION_STR).arg(edition)
            .arg(QT_VERSION_STR).arg(QSCINTILLA_VERSION_STR)
            );

//  aboutBox.setFixedSize(400,200);

  aboutBox.exec();
}

void MainWindow::showConfig()
{
  configDialog->exec(config);
}

void MainWindow::applyConfig()
{
  // apply style
  StyleManager::instance()->loadCurrentStyle(config->colorScheme);
  // apply lexers
  StyleManager::instance()->setLexerCustomFilters(config->customExt, config->customWildcard);

  // config for editors
  for (int i = 0; i < tabWidget->count(); i++)
  {
    EditWindow *ew = (EditWindow*) tabWidget->widget(i);
    ew->applyConfig(config);
  }
}

void MainWindow::navigate()
{
  int line, pos;
  textEdit->getCursorPosition(&line, &pos);

  bool ok;
  line = QInputDialog::getInt(this, tr("STE - quick navigation"),
                                  tr("Choose a line to jump to (%1 - %2):").arg(1).arg(textEdit->lines()),
                                  line+1, 1, textEdit->lines(), 5, &ok);
  if (!ok) return;

  textEdit->jumpLine(line-1, pos);
}

void MainWindow::sendActionKeys()
{
  QAction *act = dynamic_cast<QAction*>(sender());
  if (act)
  {
    int data = act->data().toUInt();
    int key = data & 0x00ffffff;
    int mod = data & 0xff000000;
    textEdit->executeKeyCommand(key, (Qt::KeyboardModifiers)mod);
  }
}

void MainWindow::defineCodecs()
{
  myCodecs << "ASCII" << "ISO-8859-1" << "ISO-8859-2" << "ISO-8859-3" << "ISO-8859-4" << "ISO-8859-5"
      << "ISO-8859-7" << "ISO-8859-9" << "ISO-8859-10" << "ISO-8859-13" << "ISO-8859-14" << "ISO-8859-15" << "ISO-8859-16"
      << "KOI8-R" << "KOI8-U" << "KOI8-RU"
      << "CP1250" << "CP1251" << "CP1252" << "CP1253" << "CP1254" << "CP1257"
      << "CP850" << "CP866"
      << "MacRoman" << "MacCentralEurope" << "MacIceland" << "MacCroatian" << "MacRomania"
      << "MacCyrillic" << "MacUkraine" << "MacGreek" << "MacTurkish" << "Macintosh"
      // Semitic languages
      << "ISO-8859-6" << "ISO-8859-8" << "CP1255" << "CP1256" << "CP862" << "MacHebrew" << "MacArabic"
      // Japanese
      << "EUC-JP" << "SHIFT_JIS" << "CP932" << "ISO-2022-JP" << "ISO-2022-JP-2" << "ISO-2022-JP-1"
      // Chinese
      << "EUC-CN" << "HZ" << "GBK" << "CP936" << "GB18030" << "EUC-TW" << "BIG5" << "CP950"
      << "BIG5-HKSCS" << "BIG5-HKSCS:2001" << "BIG5-HKSCS:1999" << "ISO-2022-CN" << "ISO-2022-CN-EXT"
      // Korean
      << "EUC-KR" << "CP949" << "ISO-2022-KR" << "JOHAB"
      // Armenian
      << "ARMSCII-8"
      //Georgian
      << "Georgian-Academy" << "Georgian-PS"
      //Tajik
      << "KOI8-T"
      //Kazakh
      << "PT154" << "RK1048"
      //Thai
      << "ISO-8859-11" << "TIS-620" << "CP874" << "MacThai"
      //Laotian
      << "MuleLao-1" << "CP1133"
      //Vietnamese
      << "VISCII" << "TCVN" << "CP1258"
      //Platform specifics
      << "HP-ROMAN8" << "NEXTSTEP"
      //Full Unicode
      << "UTF-8"
      << "UCS-2" << "UCS-2BE" << "UCS-2LE"
      << "UCS-4" << "UCS-4BE" << "UCS-4LE"
      << "UTF-16" << "UTF-16BE" << "UTF-16LE"
      << "UTF-32" << "UTF-32BE" << "UTF-32LE"
      << "UTF-7"
      << "C99" << "JAVA"
      //Full Unicode, in terms of uint16_t or uint32_t (with machine dependent endianness and alignment)
      << "UCS-2-INTERNAL" << "UCS-4-INTERNAL"
      ;

  myCodecs.sort();

  // menu
  codecSaveMenu = new QMenu(tr("Save with encoding"), this);
  connect(codecSaveMenu, SIGNAL(triggered(QAction*)), this, SLOT(saveEncoded(QAction*)));

  codecOpenMenu = new QMenu(tr("Reload with encoding"), this);
  connect(codecOpenMenu, SIGNAL(triggered(QAction*)), this, SLOT(reloadEncoded(QAction*)));

  for (int i = 0; i < myCodecs.count(); i++)
  {
    QAction *act = codecSaveMenu->addAction(myCodecs.at(i));
    act->setData(i);

    QAction *act1 = codecOpenMenu->addAction(myCodecs.at(i));
    act1->setData(i);
  }
}

void MainWindow::zoomIn()
{
  if (zoom < MAX_ZOOM) zoom++;
  zoomInAct->setEnabled(zoom < MAX_ZOOM);
  zoomOutAct->setEnabled(zoom > 1);
  textEdit->zoomTo(zoom);
}

void MainWindow::zoomOut()
{
  if (zoom > 1) zoom--;
  zoomInAct->setEnabled(zoom < MAX_ZOOM);
  zoomOutAct->setEnabled(zoom > 1);
  textEdit->zoomTo(zoom);
}

void MainWindow::zoomBase()
{
  zoom = 1;
  zoomInAct->setEnabled(zoom < MAX_ZOOM);
  zoomOutAct->setEnabled(zoom > 1);
  textEdit->zoomTo(zoom);
}

void MainWindow::showSpaces(bool)
{
  if (!textEdit)
    return;

  textEdit->setWhitespaceVisibility(
      whiteSpacesAct->isChecked() ? EditWindow::WsVisible : EditWindow::WsInvisible);
}

void MainWindow::wrapText(bool)
{
  if (!textEdit)
    return;

  textEdit->setWrapMode(wrapAct->isChecked() ? EditWindow::WrapWord : EditWindow::WrapNone);
}

void MainWindow::setEolsAction(QAction* act)
{
  QsciScintilla::EolMode mode = (QsciScintilla::EolMode) act->data().toInt();
  textEdit->setEolMode(mode);
  textEdit->convertEols(mode);

  // update status
  updateCurrentFileStatus();
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::documentInfo()
{
  #define _add(x,y) text.append(_item(tr(x), y));
  #define _addnum(x,y) text.append(_item(tr(x), QString::number(y)));

  typedef QPair<QString,QString> _item;
  QList<_item> text;

  QString docText(textEdit->text());

  _add("File", textEdit->fileName());
  _add("Modified", QFileInfo(textEdit->fileName()).lastModified().toString());
  _add("Encoding", textEdit->codec());
  _addnum("Lines", textEdit->lines());
  _addnum("Symbols in text", docText.size());
  _addnum("Symbols in memory", textEdit->length());
  //_addnum("Words", docText.count(QRegExp("(\\b)"))/2);

  // render
  QString txt("<table>");
  foreach (const _item& item, text)
  {
    txt += "<tr><td><font color=#666666>" + item.first +
           "</font></td><td width=20>&nbsp;</td><td>"
           + item.second + "</td>";
  }
  txt += "<td></table>";

  QMessageBox box(QMessageBox::Information, tr("Document Info"), txt);

  box.exec();
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::documentPreview()
{
  QString sid = styleManager->lexerSID(textEdit->lexerType());

  if (previewManager->preview(textEdit, sid, textEdit->fileName()))
    return;

  statusBar()->showMessage(tr("Preview is not available for this document type"));
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::highlightSelection()
{
  QAction *act = dynamic_cast<QAction*>(sender());
  if (!act) return;
  int id = act->data().toInt();
  textEdit->highlightSelected(id, false);
}

void MainWindow::highlightSimilarToSelection()
{
  QAction *act = dynamic_cast<QAction*>(sender());
  if (!act) return;
  int id = act->data().toInt();
  textEdit->highlightSelected(id, true);
}

void MainWindow::highlightClear()
{
  QAction *act = dynamic_cast<QAction*>(sender());
  if (!act) return;
  int id = act->data().toInt();
  textEdit->cleanIndicator(id);
}

void MainWindow::highlightClearInSelection()
{
  QAction *act = dynamic_cast<QAction*>(sender());
  if (!act) return;
  int id = act->data().toInt();
  textEdit->cleanIndicator(id, false);
}

void MainWindow::highlightClearAll()
{
  textEdit->cleanIndicator(INDICATOR_HIGHLIGHT1);
  textEdit->cleanIndicator(INDICATOR_HIGHLIGHT2);
  textEdit->cleanIndicator(INDICATOR_HIGHLIGHT3);
  textEdit->cleanIndicator(INDICATOR_HIGHLIGHT4);
  textEdit->cleanIndicator(INDICATOR_HIGHLIGHT5);
  textEdit->cleanIndicator(INDICATOR_HIGHLIGHT6);
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::showSideBar(SideBarBase *bar)
{
  static bool first = true;

  //qDebug() << "showSideBar() -------------";

  SideBarBase *old = dynamic_cast<SideBarBase*>(barStack->currentWidget());


  if (bar == old && !first)
  {
    barStack->setVisible(true);
    return;
  }

  first = false;

  if (old && barStack->isVisible())
    old->setStoredHeight(barStack->height());

  barStack->setCurrentWidget(bar);
  barStack->setVisible(true);

  //qDebug() << "storing height: " << old->storedHeight();
  //qDebug() << "setting height: " << bar->storedHeight();

  int h = bar->storedHeight();

  if (bar->isFixed())
  {
    h = bar->height();
  }
  else
    if (h <= 0) {
      h = barStack->height();
      bar->setStoredHeight(barStack->height());
    }

  barStack->resize(barStack->width(), h);
  QList<int> sizes;
  sizes << mainSplitter->height()-h-mainSplitter->handleWidth() << h;
  mainSplitter->setSizes(sizes);

  barStack->setMinimumHeight(bar->isFixed() ? h : bar->minimumHeight());
  barStack->setMaximumHeight(bar->isFixed() ? h : bar->maximumHeight());
}

void MainWindow::sideBarClosed()
{
  if (barStack->isHidden())
    return;

  //qDebug() << "sideBarClosed() -------------";

  SideBarBase *old = dynamic_cast<SideBarBase*>(barStack->currentWidget());
  if (old) {
    //qDebug() << barStack->height();
    old->setStoredHeight(barStack->height());
  }

  barStack->hide();
}

void MainWindow::addSideBar(SideBarBase *bar)
{
  barStack->addWidget(bar);
  connect(bar, SIGNAL(closed()), this, SLOT(sideBarClosed()));

  sideBars.append(bar);
}

