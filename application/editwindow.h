#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#define private protected

#include <QtGui>

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexer.h>
#include <Scintilla.h>

struct Config;
struct EditStyle;
struct TrimInfo;
struct FormatInfo;

struct Selection
{
  int lineFrom, indexFrom;
  int lineTo, indexTo;
};


class EditWindow : public QsciScintilla
{
  Q_OBJECT
public:
    EditWindow(QWidget *parent, const QString &filename = "");

    // filename
    inline const QString& fileName() const { return myFilename; }
    inline const QString& name() const { return myName; }
    QString fullName() const { return myFilename.isEmpty() ? tr("Unsaved New File") : myFilename; }

    void setFilename(const QString &filename);
    bool compareFileName(const QString &filename) const;

    inline bool justCreated() const { return myFilename.isEmpty(); }
    inline const QDateTime& lastModified() const { return myChangeTime; }

    void setLexer(QsciLexer *lexer, int lt);
    inline int lexerType() const { return myLexerType; }

    EolMode checkEols() const;

    // bookmarks
    const QList<int>& bookmarks() const { return myBookmarks; }
    QStringList bookmarkedLinesToStringList();
    void setBookmarks(const QStringList& lines);

    // folding
    enum FoldStyleEx
    {
      ArrowFoldStyle = BoxedTreeFoldStyle + 1,
      ArrowTreeFoldStyle,
      LastFoldStyle
    };

    void setFolding(int style, int margin = 2);
    void setFoldMarkersColor(const QColor& fore, const QColor& back);

    // common line/position functions
    inline int linesVisible() const { return SendScintilla(SCI_LINESONSCREEN); }
    bool isLineEmpty(int i) const;

    inline int currentLine() const { return SendScintilla(SCI_LINEFROMPOSITION, currentPos()); }
    inline int currentPos() const { return SendScintilla(SCI_GETCURRENTPOS); }
    inline void setCursorPos(int pos) { SendScintilla(SCI_GOTOPOS, pos); }

    int prevChar() const;
    int nextChar() const;

    QString textFromRange(int pos1, int pos2);

    void executeKeyCommand(int key, Qt::KeyboardModifiers mod);

    // common selections functions
    Selection currentSelection();
    inline void setCurrentSelection(const Selection &sel)
    {
      setSelection(sel.lineFrom, sel.indexFrom, sel.lineTo, sel.indexTo);
    }

    // just temp
    Selection baseSel;

    // highlighting
    void createIndicator(int id, int type, const QColor &color, bool under = true);
    void applyIndicator(int start, int end, int id);
    void applyIndicator(const QString &text, int id1, int id2 = -1);
    void cleanIndicators(int id1, int id2 = -1);

    void highlightSelected(int id, bool everywhere=false);
    void cleanIndicator(int id, bool everywhere=true);

    // commenting
    inline void setCommentMarks(const QString &mark, const QString &mark1, const QString &mark2)
    {
      mySCmark = mark; myMCmark1 = mark1; myMCmark2 = mark2;
      mySClen = mark.length(); myMClen1 = mark1.length(); myMClen2 = mark2.length();
    }

    // IO
    inline const QString& codec() const { return myCodec; }
    inline void setCodec(const QString &cod) { myCodec = cod; }
    inline bool bom() const { return myBOM; }
    inline void setBOM(bool newbom) { myBOM = newbom; }
    bool load(const QString &codec = "");
    bool save(const QString &name = "");
    bool saveEncoded(const QString &codec);

    // config
    void applyConfig(Config *pConfig);

    void applyStyle(const EditStyle &style);

    void storeSettings(QSettings &settings);
    void restoreSettings(QSettings &settings);

public slots:
    void uppercaseSelectedText();
    void lowercaseSelectedText();
    void duplicateCurrentLine();
    void joinLines(const QString &sep);
    void removeCurrentLine();
    void removeToLineEnd();
    void removeToWordEnd();
    void removeWord();
    void swapCurrentLine();
    void moveCurrentLineUpper();
    void moveCurrentLineLower();
    void fillupCurrentLine();

    void insertBefore(const QString &text);
    void replaceSelection(const QString &text);

    void spacesToTabs();
    void tabsToSpaces();
    void trimLastSpaces();
    void trimSpaces();
    void trimEmptyLines(TrimInfo &info);
    void trimEndOfLines(QsciScintilla::EolMode eols);
    void formatWords(FormatInfo &info);

    virtual void zoomTo(int size);

    void toggleBookmark();
    void toggleBookmark(int line);
    void removeAllBookmarks();
    bool navigateBookmark(int id);
    void nextBookmark();
    void prevBookmark();

    void jumpLine(int line, int pos = 0);
    void jumpBlockUp();
    void jumpBlockDown();

    int findMatchingBrace();
    void selectToMatchingBrace();

    virtual void foldAll(bool children = false);
    void foldExpandAll();
    void foldContractAll();

    void toggleComment();

protected slots:
    void handleLinesChanged();
    void checkHighlight();
    void checkHighlight(int);

    void backIndent();

protected:
    void initLexer(QsciLexer *lex);

    void defineMarkers();

    void highlightVisible(const QString &text, int id1, int id2);

    virtual void keyPressEvent (QKeyEvent *e);
    virtual void focusOutEvent (QFocusEvent *e);

    bool autoSkip(int key);
    bool autoInsert(int key);
    bool autoBraceIndent(int key);
    bool navigate(int key, Qt::KeyboardModifiers mod);
    bool autocompleterPopup(int key, Qt::KeyboardModifiers mod);

    void doCommentLine(int line);
    bool autoComment(int key, Qt::KeyboardModifiers mod);

    QString myFilename, myName;
    QDateTime myChangeTime;

    QFont myFont;
    QColor myMarginFore, myMarginBack;

    QString myCodec;
    bool myBOM;

    QList<int> myBookmarks;

    int myHLID1, myHLID2;
    QString myHLText;

    int myLexerType;

    bool myAutoPairInsertion;

    QString mySCmark, myMCmark1, myMCmark2;
    int mySClen, myMClen1, myMClen2;

    Config *myGlobalConfig;
};

#endif // EDITWINDOW_H
