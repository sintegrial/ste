#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>


#define INDICATOR_SEARCH          0
#define INDICATOR_QUICK_SEARCH1   1
#define INDICATOR_QUICK_SEARCH2   2

#define INDICATOR_HIGHLIGHT1      3
#define INDICATOR_HIGHLIGHT2      4
#define INDICATOR_HIGHLIGHT3      5
#define INDICATOR_HIGHLIGHT4      6
#define INDICATOR_HIGHLIGHT5      7
#define INDICATOR_HIGHLIGHT6      8


#define INIT_SETTINGS     QSettings settings("Sintegrial", "STE");


enum IO_State {
  IO_OK, IO_FAILED, IO_SKIP, IO_CANCEL
};


class ClickLabel;
class QsciPrinter;

class EditWindow;
class TabWidget;
class WrapDialog;
class ConfigDialog;

class StyleManager;
class FormatManager;
class ToolManager;
class ToolManagerDialog;
class PreviewManager;

struct SearchResult;
struct Config;
class RecentFiles;

class SideBarBase;
class SearchBar;
class ShortcutBar;
class ExplorerBar;
class WordFormatBar;
class SymbolsBar;
class NumbersBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

    void start();

    const QStringList& codecs() const { return myCodecs; }

public slots:
    void messageParams(const QString&);
    void loadFromParams();

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void wheelEvent(QWheelEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void newFile();

    void open();
    void openRecent(const QString &filename);
    void reload();
    void reloadEncoded(QAction* act);

    bool save();
    bool saveAll();
    bool saveAs();
    void saveEncoded(QAction* act);

    void setReadonly(bool set);

    void print();

    void about();
    void showShortcuts();

    void showConfig();
    void applyConfig();

    void zoomIn();
    void zoomOut();
    void zoomBase();
    void showSpaces(bool);
    void wrapText(bool);

    void search();
    void searchFiles();
    void searchNext();
    void searchPrev();
    void searchAll();
    void searchIncremental();
    void replace();
    void replaceAll();
    void searchClosed();
    void searchResult(const QString &file, const QString &text, const SearchResult &res);

    void closeAllWindows();
    void closeTabExcept();
    void closeTab();

    void copyTabPath();
    void copyTabName();

    void navigate();

    void trimEolMenu_activated(QAction *act);
    void trimAllEmptyLines();
    void trimBelowEmptyLines();
    void trimAboveEmptyLines();

    void formatWords();
    void formatCode(QAction* act);
    void joinLines();

    void autoTag();
    void wrapSelected();

    void formatNumber();
    void getNumber(QString &number);
    void pasteNumber(const QString &number);

    void exploreFile();

    void saveSession();
    void saveSessionAs();
    void reloadSession();
    void removeSession();
    void closeSession();
    void sessionActivated(QAction*);

    void sendActionKeys();

    void documentWasModified(bool m);
    void documentChanged();
    void linesChanged();
    void eolsChanged();
    void codecChanged();
    void bomChanged(bool bom);
    void cursorPositionChanged(int line, int pos);
    void selectionChanged();
    void updateCurrentFileStatus();
    void updateFileTab(EditWindow *edit, int idx);
    void updateFileSavedState(EditWindow *edit, int idx);
    void updateFileTabIcon(EditWindow *edit, int idx);

    void currentTabChanged(int index);
    void tabCloseRequested(int index);
    void showTabMenu(const QPoint&);
    void fillWindowsMenu();
    void switchToWindow(QAction *act);

    void setEolsAction(QAction*);

    void fillBookmarkMenu();
    void bookmarkActivated(QAction* act);
    void updateBookmarks();
    void toggleBookmarks();
    void removeBookmarks();

    void marginClicked(int,int,Qt::KeyboardModifiers);

    void highlightSelection();
    void highlightSimilarToSelection();
    void highlightClear();
    void highlightClearInSelection();
    void highlightClearAll();

    void documentInfo();

    void documentPreview();


    void lexerChanged(int lt);

    void appFocusChanged(QWidget * old, QWidget * now);

    void checkChangedFiles();

    bool loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);

    void createTab(const QString &fileName);


    void sideBarClosed();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();

    void createMainWidget();
    void createTabBar();
    void createSearchBar();
    void createTrimBar();
    void createShortcurBar();
    void createExplorerBar();
    void createSymbolsBar();
    void createWordFormatBar();
    void createNumbersBar();
    void createWrapDialog();

    void addSideBar(SideBarBase *bar);
    void showSideBar(SideBarBase *bar);

    void createManagers();

    QLayout* addStatusBarSection();
    QMenu* createHighlightMenu(const QIcon &icon, const QString &title, int id);

    void loadFilesFromArguments();
    bool loadFileSilent(const QString &fileName);

    bool doOpen(EditWindow *te, const QString &codec = "");

    bool maybeSave(int skip = -1);
    bool doSaveAll(int skip = -1, bool silent = true);

    void doSearch(bool fw);
    void doSimpleSearch(bool fw, struct SearchData data);
    void doSelectionSearch(bool fw, bool fix, struct SearchData data);
    void doDocumentSearch(bool fw, bool fix, struct SearchData data);
    void doSearchReplaceAll(bool replace);
    void showSearchResults(bool res);
    bool fixSearchData(bool fw, int *line, int *index, struct SearchData &data);
    int doOpenFilesSearch(struct SearchData data, bool replace);
    int doFileSearch(EditWindow *edit, struct SearchData data, bool replace);

    void doStoreTabs(QSettings &settings, const QString &name);
    void doReadTabs(QSettings &settings, const QString &name);

    void connectSignals();
    void disconnectSignals();

    void defineCodecs();

    void doSaveSession(const QString &name);
    void doLoadSession(const QString &name);
    void doRemoveSession(const QString &name);
    QStringList sessionList() const;
    void updateSessionActions();
    void fillSessionsMenu();

    void readSettings();
    void writeSettings();

    EditWindow* findTab(const QString &name);

    EditWindow *textEdit;
    QWidget *mainWidget;
    QSplitter *mainSplitter;
    QStackedWidget *barStack;
    QList<SideBarBase*> sideBars;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *editEolsMenu;
    QMenu *formatMenu;
    QMenu *searchMenu;
    QMenu *bookmarkMenu;
    QMenu *viewMenu;
    QMenu *sessionMenu;
    QMenu *windowsMenu;
    QMenu *helpMenu;
    QMenu *markupMenu;
    QMenu *stylesMenu;
    QMenu *toolsMenu;

    QMenu *tabMenu;
    QMenu *toolBarsMenu;
    QMenu *sessionListMenu;
    QMenu *editContextMenu;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *viewToolBar;
    QToolBar *searchToolBar;
    QToolBar *markupToolBar;
    QToolBar *trimToolBar;
    QToolBar *formatToolBar;

    QAction *newAct;
    QAction *openAct;
    QAction *reloadAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *saveAllAct;
    QAction *exitAct;

    QAction *readOnlyAct;

    QAction *printAct;

    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *undoAct;
    QAction *redoAct;

    QAction *upperAct;
    QAction *lowerAct;
    QAction *lineDuplicateAct;
    QAction *lineSwapAct;
    QAction *lineUpAct;
    QAction *lineDownAct;
    QAction *lineKillAct;
    QAction *lineStartKillAct;
    QAction *lineEndKillAct;
    QAction *wordKillAct;
    QAction *wordEndKillAct;
    QAction *wordStartKillAct;
    QAction *lineFillAct;

    QAction *tabsToSpacesAct;
    QAction *spacesToTabsAct;
    QAction *trimRightSpacesAct;
    QAction *trimSpacesAct;
    QAction *trimEmptyLinesAct;
    QAction *trimAllEmptyLinesAct;
    QAction *trimAboveEmptyLinesAct;
    QAction *trimBelowEmptyLinesAct;
    QAction *wordFormatAct;
    QAction *linesJoinAct;
    QAction *toggleCommentAct;
    QAction *autoTagAct;
    QAction *numberAct;

    QAction *winEolAct, *unixEolAct, *macEolAct;
    QActionGroup *eolsGroup;

    QAction *zoomInAct, *zoomOutAct, *zoomBaseAct;
    QAction *whiteSpacesAct;
    QAction *eolsAct;
    QAction *indentGuidesAct;
    QAction *wrapAct;
    QAction *foldAct, *foldExpandAct, *foldContractAct;
    QAction *infoAct;

    QAction *addBookmarkAct;
    QAction *removeAllBookmarksAct;
    QAction *nextBookmarkAct;
    QAction *prevBookmarkAct;

    QAction *searchAct, *searchFilesAct;
    QAction *searchNextAct, *searchPrevAct;
    QAction *searchBraceAct, *selectBraceAct;

    QAction *navigateAct;

    QAction *closeTabAct;
    QAction *closeAllExceptAct;
    QAction *closeAllAct;

    QAction *copyPathAct;
    QAction *copyNameAct;

    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *shortcutsAct;

    QAction *configAct;
    QAction *toolsAct;

    QAction *explorerAct, *explorerFileAct;

    QAction *symbolsAct;

    QAction *saveSessionAct;
    QAction *saveSessionAsAct;
    QAction *reloadSessionAct;
    QAction *removeSessionAct;
    QAction *closeSessionAct;

    QAction *previewAct;

    TabWidget *tabWidget;
    QToolButton *tabListButton;
    QMenu *tabListMenu;

    ClickLabel *linesLabel, *rowLabel, *columnLabel;
    ClickLabel *bytesLabel, *charsLabel;
    ClickLabel *eolLabel;
    ClickLabel *codecLabel;

    StyleManager *styleManager;
    FormatManager *formatManager;
    ToolManager *toolManager;
    ToolManagerDialog *toolManagerDialog;
    PreviewManager *previewManager;

    int zoom;
    QStringList externParams;

    Config *config;
    ConfigDialog *configDialog;

    RecentFiles *recentFiles;

    QsciPrinter *printer;
    QPrintDialog *printDialog;

    SearchBar *searchBar;
    bool newSearch;

    QSpinBox *trimCounter;

    ShortcutBar *shortcutBar;

    ExplorerBar *explorerBar;

    WordFormatBar *wordFormatBar;

    SymbolsBar *symbolsBar;

    NumbersBar *numbersBar;

    WrapDialog *wrapDialog;

    QStringList myCodecs;
    QMenu *codecSaveMenu, *codecOpenMenu;
    QAction *bomAct;

    QString lastFileDir;

    QString mySession;

    QList<QMenu*> highlightMenuList;
    QAction *clearAllHlAct;
  };

#endif
