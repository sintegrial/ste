#include "editwindow.h"
#include "editstyle.h"
#include "config.h"
#include "utils.h"
#include "editdefines.h"

#include "stylemanager.h"

#include <Qsci/qscilexer.h>
#include <Qsci/qsciscintilla.h>

#include <utf/Utf8_16.h>
#include <iconv.h>
#include <errno.h>

#include <enca.h>

#include <QDebug>

#define MARKER_BOOK       0
#define MARKER_BOOK_MASK  (1 << MARKER_BOOK)

#define PressKey(key, mod) { \
  QKeyEvent ev(QEvent::KeyPress, key, mod); \
  QsciScintilla::keyPressEvent(&ev); }

////////////////////////////////////////////////////////////////////////////////

EditWindow::EditWindow(QWidget *parent, const QString &filename) :
    QsciScintilla(parent),
    myBOM(false),
    myHLID1(-1), myHLID2(-1),
    myAutoPairInsertion(true),
    mySClen(0), myMClen1(0), myMClen2(0)
{
  setFilename(filename);

  setMarginType(0, NumberMargin);
  setMarginLineNumbers(0, true);

  setMarginType(1, SymbolMargin);
  setMarginSensitivity(1, true);
  setMarginMarkerMask(1, MARKER_BOOK_MASK);
  setMarginWidth(1, 24);

  defineMarkers();

  setCaretLineVisible(true);

  setIndentationGuides(true);

  QShortcut *shiftTab = new QShortcut(QKeySequence("Shift+Tab"), this);
  connect(shiftTab, SIGNAL(activated()), this, SLOT(backIndent()) );

  setAutoCompletionSource(AcsAll);
  setAutoCompletionReplaceWord(true);
  setAutoCompletionThreshold(2);
  setAutoCompletionCaseSensitivity(false);
  //setAutoCompletionShowSingle(true);

  setFolding(ArrowTreeFoldStyle, 2);

  setWrapVisualFlags(WrapFlagByText);

  setBraceMatching(SloppyBraceMatch);

  setCallTipsVisible(true);
  setCallTipsStyle(CallTipsContext);

  setUtf8(true);

  connect(this, SIGNAL(linesChanged()), this, SLOT(handleLinesChanged()));
  connect(this, SIGNAL(linesChanged()), this, SLOT(checkHighlight()));
  connect(this, SIGNAL(SCN_ZOOM()), this, SLOT(checkHighlight()));
  connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(checkHighlight(int)));

  #ifdef Q_OS_WIN32
    myFont.setFamily("courier");
  #else
    myFont.setFamily("monospace");
  #endif
    myFont.setPointSize(8);

  setLexer(0, 0);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // JUST TEST
  //setCommentMarks("//", "/*", "*/");
}

void EditWindow::applyConfig(Config *pConfig)
{
  setUpdatesEnabled(false);
  blockSignals(true);

  myGlobalConfig = pConfig;
  Config& config = *pConfig;

  setAutoIndent(config.autoIndent);
  setBackspaceUnindents(config.backspaceUnindent);
  setTabIndents(config.tabIndents);
  setTabWidth(config.tabSize);
  setIndentationWidth(config.indentSize);
  setIndentationsUseTabs(!config.spaceTabs);

  setEdgeColumn(config.lineEdgeSize);
  setEdgeMode(config.lineEdge ? EdgeLine : EdgeNone);

  setFolding(config.foldStyle, 2);

  setAutoCompletionSource((QsciScintilla::AutoCompletionSource)config.autoCompletion);
  setAutoCompletionCaseSensitivity(config.autoCompletionCaseSensitive);
  setAutoCompletionReplaceWord(config.autoCompletionReplaceWord);

  myFont.setFamily(config.font);
  myFont.setPointSize(config.fontSize);

  initLexer(lexer());

  myAutoPairInsertion = config.insertPairs;

  blockSignals(false);
  setUpdatesEnabled(true);
}

void EditWindow::applyStyle(const EditStyle &style)
{
  setUpdatesEnabled(false);

  setPaper(style.BackgroundColor);
  setColor(style.TextColor);

  setCaretLineBackgroundColor(style.CaretLineBackgroundColor);
  setCaretForegroundColor(style.CaretLineForegroundColor);

  if (style.SelectionForegroundColor != QColor(Qt::transparent))
    setSelectionForegroundColor(style.SelectionForegroundColor);
  else
    resetSelectionForegroundColor();

  setSelectionBackgroundColor(style.SelectionBackgroundColor);

  setMatchedBraceBackgroundColor(style.MatchedBraceBackgroundColor);
  setMatchedBraceForegroundColor(style.MatchedBraceForegroundColor);
  setUnmatchedBraceBackgroundColor(style.UnmatchedBraceBackgroundColor);
  setUnmatchedBraceForegroundColor(style.UnmatchedBraceForegroundColor);

  setMarginsBackgroundColor(myMarginBack = style.MarginsBackgroundColor);
  setMarginsForegroundColor(myMarginFore = style.MarginsForegroundColor);

  setFoldMarginColors(style.FoldMarginForegroundColor, style.FoldMarginBackgroundColor);
  setFoldMarkersColor(style.FoldMarkersForegroundColor, style.FoldMarkersBackgroundColor);

  setIndentationGuidesForegroundColor(style.IndentationGuidesForegroundColor);
  setIndentationGuidesBackgroundColor(style.IndentationGuidesBackgroundColor);

  setEdgeColor(style.EdgeColor);

  setUpdatesEnabled(true);
}

// back and fore swapped here!
void EditWindow::setFoldMarkersColor(const QColor& back, const QColor& fore)
{
  // folding markers
  SendScintilla(SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, fore);
  SendScintilla(SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, back);

  SendScintilla(SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, fore);
  SendScintilla(SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, back);

  SendScintilla(SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPENMID, fore);
  SendScintilla(SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPENMID, back);

  SendScintilla(SCI_MARKERSETFORE, SC_MARKNUM_FOLDERSUB, fore);
  SendScintilla(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, back);

  SendScintilla(SCI_MARKERSETFORE, SC_MARKNUM_FOLDERTAIL, fore);
  SendScintilla(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, back);

  SendScintilla(SCI_MARKERSETFORE, SC_MARKNUM_FOLDERMIDTAIL, fore);
  SendScintilla(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, back);

  SendScintilla(SCI_MARKERSETFORE, SC_MARKNUM_FOLDEREND, fore);
  SendScintilla(SCI_MARKERSETBACK, SC_MARKNUM_FOLDEREND, back);
}

void EditWindow::setFilename(const QString &filename)
{
  myFilename = Utils::normalizeFilename(filename);

  QFileInfo fi(myFilename);
  if (myFilename.isEmpty())
    myName = tr("Untitled");
  else
    myName = fi.fileName();
}

bool EditWindow::compareFileName(const QString &filename) const
{
  // assuming that filenames are normalized
#ifdef Q_OS_WIN32
  return myFilename.toLower() == filename.toLower();//.replace('/', '\\');
#else
  return myFilename == filename;
#endif
}

void EditWindow::handleLinesChanged()
{
  setMarginWidth(0, QString("_%1").arg(lines()));
}

void EditWindow::zoomTo(int size)
{
  QsciScintilla::zoomTo(size);
  handleLinesChanged();
}

void EditWindow::setLexer(QsciLexer *lex, int lt)
{
  myLexerType = lt;

  QsciLexer *old_lexer = lexer();
  QsciScintilla::setLexer(0);
  if (old_lexer)
    delete old_lexer;

  initLexer(lex);

  QsciScintilla::setLexer(lex);
}

void EditWindow::initLexer(QsciLexer *lex)
{
  // assign default style
  const EditStyle &defStyle = StyleManager::instance()->editStyle();
  applyStyle(defStyle);
  // assign lexer style
  StyleManager::instance()->applyCurrentStyle(lex, myFont);

  if (lex)
  {
    lex->setAutoIndentStyle(AiMaintain);
    lex->refreshProperties();

    const LexerInfo &li = StyleManager::instance()->lexerInfo(myLexerType);
    setCommentMarks(li.comm.markSC, li.comm.markMC1, li.comm.markMC2);
  }
  else
  {
    setCommentMarks("","","");
  }

  setFont(myFont);

  setMarginsFont(myFont);
  setMarginsForegroundColor(myMarginFore);
  setMarginsBackgroundColor(myMarginBack);
}

////////////////////////////////////////////////////////////////////////////////

QsciScintilla::EolMode EditWindow::checkEols() const
{
  const QString &str = text();

  int crlf = str.count("\r\n");
  int lf = str.count("\n") - crlf;
  int cr = str.count("\r") - crlf;

  //QMessageBox::about(0,"", QString("%1  %2  %3").arg(crlf).arg(cr).arg(lf));

  if (crlf > lf && crlf > cr)
    return EolWindows;
  if (cr > lf)
    return EolMac;
  return EolUnix;
}

////////////////////////////////////////////////////////////////////////////////

void EditWindow::uppercaseSelectedText()
{
  if (hasSelectedText())
  {
    int start, end, from, to;
    getSelection(&start, &from, &end, &to);

    QString txt = selectedText().toUpper();
    SendScintilla(SCI_TARGETFROMSELECTION);
    SendScintilla(SCI_REPLACETARGET, -1, txt.toUtf8().data());

    setSelection(start, from, end, to);
  }
}

void EditWindow::lowercaseSelectedText()
{
  if (hasSelectedText())
  {
    int start, end, from, to;
    getSelection(&start, &from, &end, &to);

    QString txt = selectedText().toLower();
    SendScintilla(SCI_TARGETFROMSELECTION);
    SendScintilla(SCI_REPLACETARGET, -1, txt.toUtf8().data());

    setSelection(start, from, end, to);
  }
}

void EditWindow::duplicateCurrentLine()
{
  beginUndoAction();

  PressKey(Qt::Key_D, Qt::ControlModifier);

  // if there is selection...
  if (hasSelectedText())
  {
    QString sel = selectedText();
    int pos = SendScintilla(SCI_GETSELECTIONEND);
    SendScintilla(SCI_SETSEL, pos, pos+sel.length());
  }

  endUndoAction();
}

void EditWindow::removeCurrentLine()
{
  QKeyEvent ev(QEvent::KeyPress, Qt::Key_L, Qt::ControlModifier);
  keyPressEvent(&ev);
}

void EditWindow::removeToWordEnd()
{
  QKeyEvent ev(QEvent::KeyPress, Qt::Key_Delete, Qt::ControlModifier);
  keyPressEvent(&ev);
}

void EditWindow::removeWord()
{
  int line, pos;
  getCursorPosition(&line, &pos);
  QString txt = text(line);
  if (!isWordCharacter(txt.at(pos).toAscii()))
  {
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    keyPressEvent(&ev);
    return;
  }

  beginUndoAction();

  QKeyEvent ev(QEvent::KeyPress, Qt::Key_Delete, Qt::ControlModifier);
  keyPressEvent(&ev);

  if (pos && isWordCharacter(txt.at(pos-1).toAscii()))
  {
    QKeyEvent ev1(QEvent::KeyPress, Qt::Key_Backspace, Qt::ControlModifier);
    keyPressEvent(&ev1);
  }

  endUndoAction();
}

void EditWindow::removeToLineEnd()
{
  QKeyEvent ev(QEvent::KeyPress, Qt::Key_Delete, Qt::ControlModifier | Qt::ShiftModifier);
  keyPressEvent(&ev);
}

void EditWindow::swapCurrentLine()
{
  QKeyEvent ev(QEvent::KeyPress, Qt::Key_T, Qt::ControlModifier);
  keyPressEvent(&ev);
}

void EditWindow::moveCurrentLineUpper()
{
  if (hasSelectedText())
  {
    int start, end, from, to;
    getSelection(&start, &from, &end, &to);
    int end2 = end;
    if (!to) end--;

    if (!start)
      return;

    setUpdatesEnabled(false);
    beginUndoAction();

    for (int i = start; i <= end; i++)
    {
      setCursorPosition(i, 0);
      swapCurrentLine();
    }

    setSelection(start-1, from, end2-1, to);

    endUndoAction();
    setUpdatesEnabled(true);
    return;
  }

  int line, index;
  getCursorPosition(&line, &index);
  swapCurrentLine();
  setCursorPosition(line-1, index);
}

void EditWindow::moveCurrentLineLower()
{
  if (hasSelectedText())
  {
    int start, end, from, to;
    getSelection(&start, &from, &end, &to);
    int end2 = end;
    if (!to) end--;

    if (end >= lines()-1)
      return;

    setUpdatesEnabled(false);
    beginUndoAction();

    for (int i = end+1; i > start; i--)
    {
      setCursorPosition(i, 0);
      swapCurrentLine();
    }

    setSelection(start+1, from, end2+1, to);

    endUndoAction();
    setUpdatesEnabled(true);
    return;
  }

  int line, index;
  getCursorPosition(&line, &index);
  setCursorPosition(line+1, 0/*index*/);
  swapCurrentLine();
  setCursorPosition(line+1, index);
}

void EditWindow::fillupCurrentLine()
{
  // fill up to the end position
  int end = edgeColumn();
  if (end <= 0)
    return;

  int idx = currentLine();

  // pattern
  QString pattern = text(idx);
  pattern.remove('\n');
  pattern.remove('\r');
  int oldLength = pattern.length();
  if (oldLength >= end)
    return;

  pattern = pattern.trimmed();
  if (pattern.isEmpty())
    return;

  if (hasSelectedText())
  {
    // pattern from selection
    pattern = selectedText().trimmed();
    if (pattern.contains('\n') || pattern.contains('\r'))
      return;
  }

  QString line;
  int count = end - oldLength;
  while (line.length() <= count)
  {
    line += pattern;
  }
  line = line.left(count);

  beginUndoAction();

  int lineEnd = SendScintilla(SCI_GETLINEENDPOSITION, idx);
  SendScintilla(SCI_INSERTTEXT, lineEnd, line.toUtf8().data());

  setCursorPosition(idx, end);

  endUndoAction();
}

void EditWindow::joinLines(const QString &sep)
{
  int lstart = currentLine();
  int lend = lstart+1;

  if (hasSelectedText())
  {
    int from, to;
    getSelection(&lstart, &from, &lend, &to);
    if (lstart == lend)
      return;
  }
  else
  {
    if (lend >= lines())
      return;

    setUpdatesEnabled(false);
    setSelection(lstart, 0, lend, 0);
  }

  setUpdatesEnabled(false);
  beginUndoAction();

  QString sel = selectedText();
  QStringList sl(sel.split("\n"));
  sel = sl.join(sep);
//  sel.remove('\n');
//  sel.remove('\r');
  replaceSelection(sel);

  endUndoAction();
  setUpdatesEnabled(true);
}

////////////////////////////////////////////////////////////////////////////////

void EditWindow::tabsToSpaces()
{
  setUpdatesEnabled(false);

  int size = tabWidth();
  QByteArray spaces;
  spaces.fill(' ', size);

  beginUndoAction();

  SendScintilla(SCI_SETTARGETSTART, 0);
  SendScintilla(SCI_SETTARGETEND, length());

  int pos;
  while ((pos = SendScintilla(SCI_SEARCHINTARGET, 1, "\t")) >= 0)
  {
    SendScintilla(SCI_REPLACETARGET, size, spaces.data());
    SendScintilla(SCI_SETTARGETSTART, pos+size);
    SendScintilla(SCI_SETTARGETEND, length());
  }

  endUndoAction();
  setUpdatesEnabled(true);
}

void EditWindow::spacesToTabs()
{
  setUpdatesEnabled(false);

  int size = tabWidth();
  QByteArray spaces;
  spaces.fill(' ', size);

  beginUndoAction();

  SendScintilla(SCI_SETTARGETSTART, 0);
  SendScintilla(SCI_SETTARGETEND, length());

  int pos;
  while ((pos = SendScintilla(SCI_SEARCHINTARGET, size, spaces.data())) >= 0)
  {
    SendScintilla(SCI_REPLACETARGET, 1, "\t");
    SendScintilla(SCI_SETTARGETSTART, pos+1);
    SendScintilla(SCI_SETTARGETEND, length());
  }

  endUndoAction();
  setUpdatesEnabled(true);
}

void EditWindow::trimLastSpaces()
{
  setUpdatesEnabled(false);
  beginUndoAction();

  int startR = 0, endR = lines()-1;
  if (hasSelectedText())
  {
    Selection sel = currentSelection();
    startR = sel.lineFrom;
    endR = sel.lineTo;
  }

  for (int i = startR; i <= endR; i++)
  {
    int start = SendScintilla(SCI_POSITIONFROMLINE, i);
    int end = SendScintilla(SCI_GETLINEENDPOSITION, i);

    int pos = end-1;
    for (; pos >= start; pos--)
    {
      char ch = SendScintilla(SCI_GETCHARAT, pos);
      if (ch != ' ' && ch != '\t')
        break;
    }

    if (pos/*+1*/ < end-1)
    {
      SendScintilla(SCI_SETTARGETSTART, pos+1);
      SendScintilla(SCI_SETTARGETEND, end);
      SendScintilla(SCI_REPLACETARGET, -1, "");
    }
  }

  endUndoAction();
  setUpdatesEnabled(true);
}

void EditWindow::trimSpaces()
{
  setUpdatesEnabled(false);
  beginUndoAction();

  int startR = 0, endR = lines()-1;

  if (hasSelectedText())
  {
    Selection sel = currentSelection();
    startR = sel.lineFrom;
    endR = sel.lineTo;
  }

  for (int i = startR; i <= endR; i++)
  {
    QString txt = text(i).simplified();
    int start = SendScintilla(SCI_POSITIONFROMLINE, i);
    int end = SendScintilla(SCI_GETLINEENDPOSITION, i);
    SendScintilla(SCI_SETTARGETSTART, start);
    SendScintilla(SCI_SETTARGETEND, end);
    SendScintilla(SCI_REPLACETARGET, -1, txt.toUtf8().data());
  }

  endUndoAction();
  setUpdatesEnabled(true);
}

void EditWindow::trimEmptyLines(TrimInfo &info)
{
  int keep = info.count;
  bool above = info.isAbove;

  setUpdatesEnabled(false);
  beginUndoAction();

  int cnt = 0;

  int startR = 0, endR = lines()-1;
  if (hasSelectedText())
  {
    Selection sel = currentSelection();
    startR = sel.lineFrom;
    endR = sel.lineTo;
  }

  // remove only lines >= keep
  if (above)
  {
    for (int i = endR; i >= startR; i--)
    {
      // line is not empty
      if (!isLineEmpty(i))
      {
        cnt = 0;
        continue;
      }

      // check if needs to remove
      if (cnt < keep)
        cnt++;
      else  // remove line
      {
        int start = SendScintilla(SCI_POSITIONFROMLINE, i);
        int end;

        // end position
        if (i == lines()-1)
          end = length();
        else
          end = SendScintilla(SCI_POSITIONFROMLINE, i+1);

        SendScintilla(SCI_SETTARGETSTART, start);
        SendScintilla(SCI_SETTARGETEND, end);
        SendScintilla(SCI_REPLACETARGET, -1, "");
      }
    }
  }
  else // remove only lines <= keep
  {
    int last = endR;

    for (int i = endR; i >= startR; i--)
    {
      if (isLineEmpty(i))
      {
        cnt++;
        continue;
      }

      // line is not empty
      if (cnt)
      {
        //if (cnt <= keep)
        {
          // remove lines from i+1 to last
          int start = SendScintilla(SCI_POSITIONFROMLINE, i+1);
          int end;

          // lines to remove
          if (cnt > keep)
            last = i+keep+1;

          // end position
          if (last >= lines()-1)
            end = length();
          else
            end = SendScintilla(SCI_POSITIONFROMLINE, last);

          //qDebug() << "remove: " << i+1 << '-' << last;

          SendScintilla(SCI_SETTARGETSTART, start);
          SendScintilla(SCI_SETTARGETEND, end);
          SendScintilla(SCI_REPLACETARGET, -1, "");
        }

        // update counters
        cnt = 0;
      }

      last = i;
    }
  }

  endUndoAction();
  setUpdatesEnabled(true);
}

void EditWindow::trimEndOfLines(QsciScintilla::EolMode eols)
{
  setUpdatesEnabled(false);
  beginUndoAction();

  int startR = 0, endR = lines()-1;
  if (hasSelectedText())
  {
    Selection sel = currentSelection();
    startR = sel.lineFrom;
    endR = sel.lineTo;
  }

  while (startR <= endR)
  {
    int pos = SendScintilla(SCI_GETLINEENDPOSITION, startR);
    startR++;

    char ch1 = SendScintilla(SCI_GETCHARAT, pos);
    char ch2 = SendScintilla(SCI_GETCHARAT, pos+1);

//    qDebug() << "--- " << startR;
//    qDebug() << pos;
//    qDebug() << (int) ch1 << "," << (int) ch2;

    if (ch1 == '\r' && ch2 == '\n')
    {
      if (eols == EolWindows)
      {
        SendScintilla(SCI_SETTARGETSTART, pos);
        SendScintilla(SCI_SETTARGETEND, pos+2);
        SendScintilla(SCI_REPLACETARGET, -1, "");
        startR--;
        endR--;
      }
      continue;
    }

    if ((ch1 == '\r' && eols == EolMac) || (ch1 == '\n' && eols == EolUnix))
    {
      SendScintilla(SCI_SETTARGETSTART, pos);
      SendScintilla(SCI_SETTARGETEND, pos+1);
      SendScintilla(SCI_REPLACETARGET, -1, "");
      startR--;
      endR--;
    }

  }

  endUndoAction();
  setUpdatesEnabled(true);
}

void EditWindow::formatWords(FormatInfo &info)
{
  QString txt(hasSelectedText() ? selectedText() : text());
  if (txt.isEmpty())
    return;

  // split by whitespaces
  QStringList sl = txt.split(QRegExp("\\s+"));
  if (sl.isEmpty())
    return;

  QString res;
  QString fill = info.fill.isEmpty() ? " " : info.fill;

  // cell alignment
  if (info.count && (info.cellExpand || info.cellWidth > 0))
  {
    QList<QList<int> > table;
    int cnt = 0;
    QList<int> tmp;
    for (int i = 0; i < sl.count(); i++)
    {
      tmp.append(sl.at(i).length());
      cnt++;
      if (cnt >= info.count)
      {
        table.append(tmp);
        tmp.clear();
        cnt = 0;
      }
    }
    if (tmp.size())
    {
      while (tmp.size() < info.count)
        tmp.append(0);
      table.append(tmp);
    }

    QList<int> wmax;
    for (int i = 0; i < info.count; i++)
    {
      int max = info.cellWidth ? info.cellWidth : 0;
      for (int j = 0; j < table.size(); j++)
      {
        const QList<int> &tmp = table.at(j);
        int sz = tmp.at(i) + 1;
        if (sz > max)
          max = sz;
      }
      wmax.append(max);
    }

    int maxlen = 0;
    for (int i = 0; i < wmax.size(); i++)
      maxlen += wmax.at(i);

    cnt = 0;
    for (int j = 0; j < table.size(); j++)
    {
      QString str;
      while (str.length() < maxlen) str += info.fill;

      // just left align for now
      int pos = 0, rpos = 0, lastpos = 0;
      const QList<int> &tmp = table.at(j);
      for (int i = 0; i < tmp.length(); i++)
      {
        int strlen = tmp.at(i);
        if (!strlen) break;
        str = str.replace(pos, strlen, sl.at(cnt));
        cnt++;

        rpos += info.cellExpand ? wmax.at(i) : info.cellWidth;
        pos += strlen;
        lastpos = pos;

        if (pos < rpos)
          pos = rpos;
        else
          pos++;
      }

      // cleanup the end of the string
      str = str.left(lastpos);

      res += str;
      if (j < table.size()-1)
        res += "\n";
    }

  }
  else // normal alignment
  {
    int cnt = 0;
    for (int i = 0; i < sl.count(); i++)
    {
      if (cnt)
        res += fill; // here put the valid whitespace

      res += sl.at(i);
      cnt++;

      // wrap next line
      if (info.count && cnt >= info.count)
      {
        cnt = 0;
        res += "\n";
      }
    }
  }

  // replace text
  setUpdatesEnabled(false);
  beginUndoAction();

  if (!hasSelectedText())
    selectAll();

  replaceSelection(res);

  endUndoAction();
  setUpdatesEnabled(true);
}

////////////////////////////////////////////////////////////////////////////////

void EditWindow::backIndent()
{
  setUpdatesEnabled(false);
  beginUndoAction();

  if (hasSelectedText())
  {
    int l1,i1,l2,i2;
    getSelection(&l1, &i1, &l2, &i2);
    if (i2 == 0) l2--;
    for (int i = l1; i <= l2; i++)
    {
      int ind = SendScintilla(SCI_GETLINEINDENTATION, i);
      if (ind > 0)
        SendScintilla(SCI_SETLINEINDENTATION, i, ind-indentationWidth());
    }
  }
  else
  {
    int pos = SendScintilla(SCI_GETCURRENTPOS);
    int line = SendScintilla(SCI_LINEFROMPOSITION, pos);
    int ind = SendScintilla(SCI_GETLINEINDENTATION, line);
    if (ind > 0)
      SendScintilla(SCI_SETLINEINDENTATION, line, ind-indentationWidth());
  }

  endUndoAction();
  setUpdatesEnabled(true);
}

////////////////////////////////////////////////////////////////////////////////

int EditWindow::findMatchingBrace()
{
  int pos1 = currentPos();
  int origPos = pos1;
  moveToMatchingBrace();
  int pos2 = currentPos();
  if (pos1 != pos2)
    return pos1;

  // search back and forth
  while (--pos1 >= 0)
  {
    int ch = SendScintilla(SCI_GETCHARAT, pos1);
    if (ch == '(' || ch == '{' || ch == '[' || ch == '<')
    {
      setCursorPos(pos1);
      moveToMatchingBrace();
      pos2 = currentPos();
      if (pos1 != pos2)
        return pos1;
    }
  }

  while (++pos2 < length())
  {
    int ch = SendScintilla(SCI_GETCHARAT, pos2);
    if (ch == ')' || ch == '}' || ch == ']' || ch == '>')
    {
      setCursorPos(pos2);
      moveToMatchingBrace();
      pos1 = currentPos();
      if (pos1 != pos2)
        return pos2;
    }
  }

  setCursorPos(origPos);
  return -1;
}

void EditWindow::selectToMatchingBrace()
{
  int pos1 = findMatchingBrace();
  if (pos1 < 0)
    return;
  int pos2 = currentPos();
  SendScintilla(SCI_SETSEL, pos2, pos1);
}

////////////////////////////////////////////////////////////////////////////////

void EditWindow::jumpLine(int line, int pos)
{
  setCursorPosition(line, pos);
  ensureLineVisible(line);
  ensureCursorVisible();
}

void EditWindow::jumpBlockUp()
{
  if (lines() <= 1)
    return;

  int i = currentLine();

  while (i > 0 && !isLineEmpty(i))
    i--;

  while (i > 0 && isLineEmpty(i))
    i--;

  while (i > 0 && !isLineEmpty(i))
    i--;

  if (isLineEmpty(i)) i++;

  jumpLine(i);
}

void EditWindow::jumpBlockDown()
{
  if (lines() <= 1)
    return;

  int i = currentLine();
  int total = lines();

  while (i < total && !isLineEmpty(i))
    i++;

  while (i < total && isLineEmpty(i))
    i++;

  if (isLineEmpty(i)) i--;

  jumpLine(i);
}

bool EditWindow::navigate(int key, Qt::KeyboardModifiers mod)
{
  bool ctrl = (mod == Qt::ControlModifier);

  switch (key)
  {
  case Qt::Key_PageUp:
    if (ctrl)
    {
      jumpBlockUp();
      return true;
    }
    break;

  case Qt::Key_PageDown:
    if (ctrl)
    {
      jumpBlockDown();
      return true;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool EditWindow::autoSkip(int key)
{
  if (myAutoPairInsertion)
  {
    // check if need to skip the symbol
    switch (key)
    {
    case Qt::Key_BraceRight:
    case Qt::Key_BracketRight:
    case Qt::Key_ParenRight:
    case Qt::Key_QuoteDbl:
//    case Qt::Key_Apostrophe:
      {
        if (nextChar() == key)
        {
          PressKey(Qt::Key_Right, 0);
          return true;
        }
      }
      break;

    // check backspace
    case Qt::Key_Backspace:
      {
        int prevKey = prevChar();
        int nextKey = nextChar();

        if ((prevKey == '{' && nextKey == '}') ||
            (prevKey == '[' && nextKey == ']') ||
            (prevKey == '<' && nextKey == '>') ||
            (prevKey == '(' && nextKey == ')') ||
            (prevKey == '\'' && nextKey == '\'') ||
            (prevKey == '\"' && nextKey == '\"') )
        {
          PressKey(Qt::Key_Backspace, 0);
          PressKey(Qt::Key_Delete, 0);
          return true;
        }
      }
      break;

    }

  }

  return false;
}

bool EditWindow::autoInsert(int key)
{
  if (myAutoPairInsertion)
  {
    // single insertion
    if (key == Qt::Key_QuoteDbl || key == Qt::Key_Apostrophe)
    {
      int pos = SendScintilla(SCI_GETCURRENTPOS);
      if (pos > 1)
      {
//        int prevKey = SendScintilla(SCI_GETCHARAT, pos-1);
//        int prevKey2 = SendScintilla(SCI_GETCHARAT, pos-2);
        int prevKey = DirectCharAt(pos-1);
        int prevKey2 = DirectCharAt(pos-2);
        if (prevKey == key && prevKey2 == key)
          return true;
      }
    }

    // insert double symbols
    switch (key)
    {
    case Qt::Key_BraceLeft:
      insert("}");
      return true;

    case Qt::Key_BracketLeft:
      insert("]");
      return true;

    case Qt::Key_ParenLeft:
      insert(")");
      return true;

    case Qt::Key_QuoteDbl:
      insert("\"");
      return true;

//    case Qt::Key_Apostrophe:
//      insert("'");
//      return true;

// #### should be done for xml/html
//    case Qt::Key_Less:
//      insert(">");
//      return true;

    default:;
    }

  }

  return false;
}

bool EditWindow::autoBraceIndent(int key)
{
  if (!autoIndent())
    return false;

  if (key == Qt::Key_Enter || key == Qt::Key_Return)
  {
    // check if between braces
    if (prevChar() == '{' /*&& nextChar() == '}'*/)
    {
      beginUndoAction();
      PressKey(Qt::Key_Return, 0);
      PressKey(Qt::Key_Return, 0);
      PressKey(Qt::Key_Up, 0);
      if (indentationsUseTabs())
        insert("\t");
      else
        insert(QString().fill(' ', indentationWidth()));
      PressKey(Qt::Key_End, 0);
      endUndoAction();
      return true;
    }
  }

  return false;
}

bool EditWindow::autocompleterPopup(int key, Qt::KeyboardModifiers mod)
{
  if (key == ' ' && (mod == Qt::ControlModifier))
  {
    switch (autoCompletionSource())
    {
    case AcsAll:
      autoCompleteFromAll();
      break;
    case AcsAPIs:
      autoCompleteFromAPIs();
      break;
    case AcsDocument:
      autoCompleteFromDocument();
      break;
    default:;
    }
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

int EditWindow::prevChar() const
{
  int pos = SendScintilla(SCI_GETCURRENTPOS);
  if (pos == 0)
    return -1;
//  return SendScintilla(SCI_GETCHARAT, pos-1);
  return DirectCharAt(pos-1);
}

int EditWindow::nextChar() const
{
  int pos = SendScintilla(SCI_GETCURRENTPOS);
  if (pos == 0)
    return -1;
  //return SendScintilla(SCI_GETCHARAT, pos);
  return DirectCharAt(pos);
}

bool EditWindow::isLineEmpty(int i) const
{
  int start = SendScintilla(SCI_POSITIONFROMLINE, i);
  int end = SendScintilla(SCI_GETLINEENDPOSITION, i);

  for (int j = start; j < end; j++)
  {
    //char ch = SendScintilla(SCI_GETCHARAT, j);
    char ch = DirectCharAt(j);
    if (ch != ' ' && ch != '\t')
    {
      return false;
    }
   }

  return true;
}

Selection EditWindow::currentSelection()
{
  Selection sel;
  getSelection(&sel.lineFrom, &sel.indexFrom, &sel.lineTo, &sel.indexTo);
  return sel;
}

void EditWindow::replaceSelection(const QString &text)
{
  QByteArray ba(text.toUtf8());
  SendScintilla(SCI_REPLACESEL, ba.data());
}

QString EditWindow::textFromRange(int pos1, int pos2)
{
  TextRange tr;
  tr.chrg.cpMin = pos1;
  tr.chrg.cpMax = pos2;
  tr.lpstrText = new char[pos2 - pos1 + 1];
  SendScintilla(SCI_GETTEXTRANGE, 0, &tr);
  QString str = QString::fromUtf8(tr.lpstrText, pos2 - pos1);
  delete[] tr.lpstrText;
  return str;
}

void EditWindow::insertBefore(const QString &text)
{
  QByteArray ba(text.toUtf8());
  int pos = SendScintilla(SCI_GETCURRENTPOS);
  SendScintilla(SCI_INSERTTEXT, pos, ba.data());
  pos += ba.size();
  SendScintilla(SCI_SETCURRENTPOS, pos);
}

////////////////////////////////////////////////////////////////////////////////
// events

void EditWindow::keyPressEvent(QKeyEvent *e)
{
  int key = e->key();

  // check for autocompleter popup
  if (autocompleterPopup(key, e->modifiers()))
    return;

  if (autoSkip(key))
    return;

  if (autoBraceIndent(key))
    return;

  if (autoComment(key, e->modifiers()))
    return;

  QsciScintilla::keyPressEvent(e);

  if (autoInsert(key))
    return;

  if (navigate(key, e->modifiers()))
    return;
}

void EditWindow::executeKeyCommand(int key, Qt::KeyboardModifiers mod)
{
  QKeyEvent ev(QEvent::KeyPress, key, mod);
  keyPressEvent(&ev);
}

////////////////////////////////////////////////////////////////////////////////

void EditWindow::focusOutEvent (QFocusEvent *)
{
  SendScintilla(SCI_AUTOCCANCEL);
}

////////////////////////////////////////////////////////////////////////////////

void EditWindow::defineMarkers()
{
//  markerDefine(QsciScintilla::Circle, MARKER_BOOK);
//  setMarkerBackgroundColor(Qt::green, MARKER_BOOK);
//  setMarkerForegroundColor(Qt::black, MARKER_BOOK);
  markerDefine(QPixmap(":/images/ledgreen.png")
               .scaled(40,16, Qt::KeepAspectRatio, Qt::SmoothTransformation),
               MARKER_BOOK);
}

////////////////////////////////////////////////////////////////////////////////
// folding

void EditWindow::setFolding(int style, int margin)
{
  QsciScintilla::setFolding(style >= ArrowFoldStyle ? PlainFoldStyle : (FoldStyle)style,
                            margin);

  switch (style)
  {
  case ArrowFoldStyle:
    setFoldMarker(SC_MARKNUM_FOLDER, SC_MARK_ARROW);
    setFoldMarker(SC_MARKNUM_FOLDEROPEN, SC_MARK_ARROWDOWN);
    setFoldMarker(SC_MARKNUM_FOLDERSUB);
    setFoldMarker(SC_MARKNUM_FOLDERTAIL);
    setFoldMarker(SC_MARKNUM_FOLDEREND);
    setFoldMarker(SC_MARKNUM_FOLDEROPENMID);
    setFoldMarker(SC_MARKNUM_FOLDERMIDTAIL);
    break;

  case ArrowTreeFoldStyle:
    setFoldMarker(SC_MARKNUM_FOLDER, SC_MARK_ARROW);
    setFoldMarker(SC_MARKNUM_FOLDEROPEN, SC_MARK_ARROWDOWN);
    setFoldMarker(SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    setFoldMarker(SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
    setFoldMarker(SC_MARKNUM_FOLDEREND, SC_MARK_ARROW);
    setFoldMarker(SC_MARKNUM_FOLDEROPENMID, SC_MARK_ARROWDOWN);
    setFoldMarker(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
    break;

  default:;
  }
}

void EditWindow::foldAll(bool children)
{
  int line = currentLine();

  QsciScintilla::foldAll(children);

  bool vis = SendScintilla(SCI_GETLINEVISIBLE, line);
  if (vis)
    ensureLineVisible(line);
}

void EditWindow::foldContractAll()
{
  // expand
  clearFolds();

  foldAll();
}

void EditWindow::foldExpandAll()
{
  int line = currentLine();

  clearFolds();

  bool vis = SendScintilla(SCI_GETLINEVISIBLE, line);
  if (vis)
    ensureLineVisible(line);
}

////////////////////////////////////////////////////////////////////////////////
// bookmarks

void EditWindow::toggleBookmark()
{
  int line, index;
  getCursorPosition(&line, &index);

  toggleBookmark(line);
}

void EditWindow::toggleBookmark(int line)
{
  if (line < 0) line = 0;
  else if (line >= lines()) line = lines()-1;

  if (markersAtLine(line) & MARKER_BOOK_MASK)
  {
    for (int i = 0; i < myBookmarks.size(); i++)
    {
      int id = myBookmarks.at(i);
      int xline = markerLine(id);
      if (xline == line)
      {
        markerDeleteHandle(id);
        myBookmarks.removeAt(i);
        i--;
      }
    }
    return;
  }

  int id = markerAdd(line, MARKER_BOOK);
  myBookmarks.append(id);
}

void EditWindow::removeAllBookmarks()
{
  markerDeleteAll(MARKER_BOOK);
  myBookmarks.clear();
}

bool EditWindow::navigateBookmark(int id)
{
  int line = markerLine(id);
  if (line < 0)
    return false;

  setCursorPosition(line, 0);
  ensureLineVisible(line);

  return true;
}

void EditWindow::nextBookmark()
{
  int line, index;
  getCursorPosition(&line, &index);

  line = markerFindNext(line+1, MARKER_BOOK_MASK);
  if (line >= 0)
  {
    setCursorPosition(line, 0);
    ensureLineVisible(line);
  }
  else
  {
    // check from start
    line = markerFindNext(0, MARKER_BOOK_MASK);
    if (line >= 0)
    {
      setCursorPosition(line, 0);
      ensureLineVisible(line);
    }
  }
}

void EditWindow::prevBookmark()
{
  int line, index;
  getCursorPosition(&line, &index);

  line = markerFindPrevious(line-1, MARKER_BOOK_MASK);
  if (line >= 0)
  {
    setCursorPosition(line, 0);
    ensureLineVisible(line);
  }
  else
  {
    // check from end
    line = markerFindPrevious(lines(), MARKER_BOOK_MASK);
    if (line >= 0)
    {
      setCursorPosition(line, 0);
      ensureLineVisible(line);
    }
  }
}

QStringList EditWindow::bookmarkedLinesToStringList()
{
  QStringList list;
  for (int i = 0; i < myBookmarks.size(); i++)
  {
    int id = myBookmarks.at(i);
    int xline = markerLine(id);
    list.append(QString::number(xline));
  }
  return list;
}

void EditWindow::setBookmarks(const QStringList& lines)
{
  removeAllBookmarks();
  for (int i = 0; i < lines.size(); i++)
  {
    int line = lines.at(i).toInt();
    toggleBookmark(line);
  }
}

////////////////////////////////////////////////////////////////////////////////
// highlight

void EditWindow::createIndicator(int id, int type, const QColor &color, bool under)
{
  //int bgr = color.blue() | (color.green() << 8) | (color.red() << 16);
  SendScintilla(SCI_INDICSETSTYLE, id, type);
  SendScintilla(SCI_INDICSETFORE, id, color);
  //SendScintilla(SCI_INDICSETALPHA, id, color.alpha());
  SendScintilla(SCI_INDICSETALPHA, id, 100);
  SendScintilla(SCI_INDICSETUNDER, id, under);
}

void EditWindow::applyIndicator(int start, int end, int id)
{
  SendScintilla(SCI_SETINDICATORCURRENT, id);
  SendScintilla(SCI_INDICATORFILLRANGE, start, end-start);
}

void EditWindow::cleanIndicators(int id1, int id2)
{
  SendScintilla(SCI_SETINDICATORCURRENT, id1);
  SendScintilla(SCI_INDICATORCLEARRANGE, 0, length());

  if (id2 >= 0)
  {
    SendScintilla(SCI_SETINDICATORCURRENT, id2);
    SendScintilla(SCI_INDICATORCLEARRANGE, 0, length());
  }

  if (id1 == myHLID1)
  {
    myHLID1 = -1;
  }

  if (id2 == myHLID2)
  {
    myHLID2 = -1;
  }
}

void EditWindow::applyIndicator(const QString &text, int id1, int id2)
{
  cleanIndicators(id1);
  cleanIndicators(id2);

  myHLText = text;
  myHLID1 = id1;
  myHLID2 = id2;
  highlightVisible(text, id1, id2);
}

void EditWindow::checkHighlight()
{
  if (myHLID1 >= 0)
    highlightVisible(myHLText, myHLID1, myHLID2);
}

void EditWindow::checkHighlight(int)
{
  checkHighlight();
}

void EditWindow::highlightVisible(const QString &text, int id1, int id2)
{
  if (text.simplified().isEmpty())
    return;

  int line1 = firstVisibleLine();   // visual coords
  line1 = SendScintilla(SCI_DOCLINEFROMVISIBLE, line1);   // document coords
  if (line1 < 0) line1 = 0;
  int line2 = line1 + linesVisible() + 2;

  for (int i = line1; i <= line2; i++)
  {
    if (SendScintilla(SCI_GETLINEVISIBLE, i) == false)
      line2++;
  }

  //qDebug() << line1 << "-" << line2;

  int pos1 = positionFromLineIndex(line1, 0);
  if (pos1 < 0) pos1 = 0;
  int pos2 = positionFromLineIndex(line2, 0);
  if (pos2 < 0) pos2 = length();

  int flags = 0;
  TextToFind ttf;
  ttf.chrg.cpMin = pos1;
  ttf.chrg.cpMax = pos2;

  TextRange tr;
  char buf[text.length() * 2 + 2];

  int res;
  QByteArray ba = text.toUtf8();
  do {
    ttf.lpstrText = ba.data();
    res = SendScintilla(SCI_FINDTEXT, flags, &ttf);
    if (res < 0)
      break;

    ttf.chrg.cpMin = ttf.chrgText.cpMax;

    // markup
    if (id2 < 0)
      SendScintilla(SCI_SETINDICATORCURRENT, id1);
    else {
      tr.chrg = ttf.chrgText;
      tr.lpstrText = buf;
      SendScintilla(SCI_GETTEXTRANGE, 0, &tr);
      QString txt = QString::fromUtf8(tr.lpstrText);
//      qDebug() << text;
//      qDebug() << txt;
      SendScintilla(SCI_SETINDICATORCURRENT, txt == text ? id1 : id2);
    }

    SendScintilla(SCI_INDICATORFILLRANGE, ttf.chrgText.cpMin, ttf.chrgText.cpMax-ttf.chrgText.cpMin);

  } while (res >= 0);

  // remove selection
  pos1 = SendScintilla(SCI_GETSELECTIONSTART);
  pos2 = SendScintilla(SCI_GETSELECTIONEND);
  SendScintilla(SCI_INDICATORCLEARRANGE, pos1, pos2-pos1);
}

void EditWindow::highlightSelected(int id, bool everywhere)
{
  if (hasSelectedText())
  {
    SendScintilla(SCI_SETINDICATORCURRENT, id);
    int selstart = SendScintilla(SCI_GETSELECTIONSTART);
    int selend = SendScintilla(SCI_GETSELECTIONEND);
    int selsize = selend - selstart;

    if (everywhere)
    {
      setUpdatesEnabled(false);

      TextToFind ttf;
      ttf.chrg.cpMin = 0;
      ttf.chrg.cpMax = length();
      int flags = 0;
      int res;
      QByteArray ba = selectedText().toUtf8();
      do {
        ttf.lpstrText = ba.data();
        res = SendScintilla(SCI_FINDTEXT, flags, &ttf);
        if (res < 0)
          break;
        ttf.chrg.cpMin = ttf.chrgText.cpMax+1;
        // markup
        applyIndicator(ttf.chrgText.cpMin, ttf.chrgText.cpMax, id);
      } while (res >= 0);

      setUpdatesEnabled(true);
    }
    else
    {
      SendScintilla(SCI_INDICATORFILLRANGE, selstart, selsize);
    }
  } else
    cleanIndicator(id);
}

void EditWindow::cleanIndicator(int id, bool everywhere)
{
  SendScintilla(SCI_SETINDICATORCURRENT, id);
  if (everywhere)
  {
    SendScintilla(SCI_INDICATORCLEARRANGE, 0, length());
  }
  else if (hasSelectedText())
  {
    int selstart = SendScintilla(SCI_GETSELECTIONSTART);
    int selend = SendScintilla(SCI_GETSELECTIONEND);
    int selsize = selend - selstart;
    SendScintilla(SCI_INDICATORCLEARRANGE, selstart, selsize);
  }
}

////////////////////////////////////////////////////////////////////////////////
// commenting

void EditWindow::toggleComment()
{
  if (!mySClen && !myMClen1) return;

  // no selection: comment only current line
  if (!hasSelectedText())
  {
    doCommentLine(currentLine());
    return;
  }

  // comment out the selection
  QString selText = selectedText();
  QString selTextTrimmed = selText.trimmed();
  if (selTextTrimmed.isEmpty())
    return;

  Selection sel = currentSelection();

  int pos_start = positionFromLineIndex(sel.lineFrom, sel.indexFrom);
  int pos_end = positionFromLineIndex(sel.lineTo, sel.indexTo);

  // check if it is double commented block - to do before single check!
  if (myMClen1 && myMClen2)
  {
    // comment exists? remove?
    if (selTextTrimmed.startsWith(myMCmark1) && selTextTrimmed.endsWith(myMCmark2))
    {
      beginUndoAction();

      int idx1 = selText.indexOf(myMCmark1);
      selText.remove(idx1, myMClen1);
      int idx2 = selText.lastIndexOf(myMCmark2);
      selText.remove(idx2, myMClen2);

      SendScintilla(SCI_TARGETFROMSELECTION);
      SendScintilla(SCI_REPLACETARGET, -1, selText.toUtf8().data());
      SendScintilla(SCI_SETSEL, SendScintilla(SCI_GETTARGETSTART), SendScintilla(SCI_GETTARGETEND));

      endUndoAction();
      return;
    }
  }

  // check if this block can be single commented
  if (mySClen)
  {
    bool empty_start = false, empty_end = false;

    if (sel.indexFrom == 0)
      empty_start = true;
    else {
      empty_start = textFromRange(
          positionFromLineIndex(sel.lineFrom, 0),
          pos_start
          ).trimmed().isEmpty();
    }

    if (sel.indexTo == 0)
      empty_end = true;
    else {
      empty_end = textFromRange(
          pos_end,
          positionFromLineIndex(sel.lineTo+1, 0)
          ).trimmed().isEmpty();
    }

    if (empty_start && empty_end)
    {
      beginUndoAction();

      // corrections
      if (sel.indexTo == 0) sel.lineTo--;
      if (isLineEmpty(sel.lineFrom)) { sel.lineFrom++; sel.indexFrom = 0; }
      // a workaround: move cursor to the next line to replace EOL as well
      setSelection(sel.lineFrom, 0, sel.lineTo+1, 0);

      QStringList sl;
      for (int i = sel.lineFrom; i <= sel.lineTo; i++)
        sl += text(i);

//      qDebug() << sl.count();
//      qDebug() << sl;

      bool comm = false;
      for (int i = 0; i < sl.count(); i++)
        if (!sl.at(i).trimmed().startsWith(mySCmark))
        {
          comm = true;
          break;
        }

      for (int i = 0; i < sl.count(); i++)
      {
        if (comm)
          sl[i] = sl[i].prepend(mySCmark);
        else {
          int idx = sl.at(i).indexOf(mySCmark);
          sl[i] = sl[i].remove(idx, mySClen);
        }
      }

      SendScintilla(SCI_TARGETFROMSELECTION);
      SendScintilla(SCI_REPLACETARGET, -1, sl.join("").toUtf8().data());
      SendScintilla(SCI_SETSEL, SendScintilla(SCI_GETTARGETSTART), SendScintilla(SCI_GETTARGETEND));

      endUndoAction();
      return;
    }
  }

  // else, set double comment
  if (myMClen1 && myMClen2)
  {
    beginUndoAction();

    // last is first
    SendScintilla(SCI_INSERTTEXT, pos_end, myMCmark2.toUtf8().data());
    SendScintilla(SCI_INSERTTEXT, pos_start, myMCmark1.toUtf8().data());

    // select everything
    setSelection(sel.lineFrom, sel.indexFrom, sel.lineTo, sel.indexTo + myMClen2);

    endUndoAction();
    return;
  }
}

void EditWindow::doCommentLine(int line)
{
  if (isLineEmpty(line))
    return;

  QString selText = text(line);
  selText.remove("\n"); selText.remove("\r");
  QString selTextTrimmed = selText.trimmed();
  int pos_start = SendScintilla(SCI_POSITIONFROMLINE, line);
  int pos_end = SendScintilla(SCI_GETLINEENDPOSITION, line);

  // comment exists? remove?
  if (myMClen1 && myMClen2)
  {
    if (selTextTrimmed.startsWith(myMCmark1) && selTextTrimmed.endsWith(myMCmark2))
    {
      beginUndoAction();

      int idx1 = selText.indexOf(myMCmark1);
      selText.remove(idx1, myMClen1);
      int idx2 = selText.lastIndexOf(myMCmark2);
      selText.remove(idx2, myMClen2);

      SendScintilla(SCI_SETTARGETSTART, pos_start);
      SendScintilla(SCI_SETTARGETEND, pos_end);
      SendScintilla(SCI_REPLACETARGET, -1, selText.toUtf8().data());

      endUndoAction();
      return;
    }
  }

  // check for single comments
  if (mySClen)
  {
    if (selTextTrimmed.startsWith(mySCmark))
    { // remove comment
      int idx = selText.indexOf(mySCmark);
      selText = selText.remove(idx, mySClen);
    } else
    { // set comment
      selText = selText.prepend(mySCmark);
    }

    //qDebug() << selText;

    SendScintilla(SCI_SETTARGETSTART, pos_start);
    SendScintilla(SCI_SETTARGETEND, pos_end);
    SendScintilla(SCI_REPLACETARGET, -1, selText.toUtf8().data());
    return;
  }

  // else check for multi comments
  if (myMClen1 && myMClen2)
  {
    beginUndoAction();

    // last is first
    SendScintilla(SCI_INSERTTEXT, pos_end, myMCmark2.toUtf8().data());
    SendScintilla(SCI_INSERTTEXT, pos_start, myMCmark1.toUtf8().data());

    endUndoAction();
    return;
  }
}

bool EditWindow::autoComment(int key, Qt::KeyboardModifiers mod)
{
  if (key == '/' && mod == Qt::ControlModifier)
  {
    toggleComment();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// io

bool EditWindow::load(const QString &codec)
{
  bool loaded = false;

  // do before open!
  bool wr = QFileInfo(myFilename).isWritable();

  // check file
  QFile file(myFilename);
  if (!file.open(QFile::ReadOnly))
    return false;

  //qDebug() << wr;
  setReadOnly(!wr);

  myCodec = codec;

  // autodetect
  if (myCodec.isEmpty() || myCodec.startsWith("UTF", Qt::CaseInsensitive))
  {
    // read 1st 10 bytes to check
    char buf[10];
    int len = file.read(buf, 10);

    // first, test for UTF
    myBOM = true;
    Utf8_16_Read utfread;
    /*int size = */utfread.convert(buf, len);
    switch (utfread.getEncoding())
    {
    case Utf8_16::eUtf8:
      myCodec = "UTF-8";
      break;
    case Utf8_16::eUtf16BigEndian:
      myCodec = "UTF-16BE";
      break;
    case Utf8_16::eUtf16LittleEndian:
      myCodec = "UTF-16LE";
      break;
    default:
      myBOM = false;
    }

    file.seek(0);
  }

  // continue if not detected
  if (myCodec.isEmpty())
  {
    char buf[10000];
    int len = file.read(buf, 10000);
    //qDebug() << len;
    file.seek(0);
    EncaAnalyser enca = enca_analyser_alloc("ru");
    EncaEncoding res = enca_analyse(enca, (unsigned char*)buf, len);
    QString name( enca_charset_name(res.charset, ENCA_NAME_STYLE_ICONV) );
    enca_analyser_free(enca);

    //qDebug() << "enca: " << name;

    if (!name.isEmpty() && name != "???")
      myCodec = name;
  }

  if (myCodec.isEmpty())
    myCodec = "ANSI";

  // codec is in myCodec
  QTextStream in(&file);
  if (myCodec == "ANSI" || myCodec == "ASCII")
  {
    myBOM = false;
    setText(in.readAll());
    loaded = true;
  }
  else
  {
    in.setCodec(myCodec.toAscii().data());
    //qDebug() << "load codec: " << in.codec()->name();

    if (in.codec()->name().toLower() != "system")
    {
      setText(in.readAll());
      loaded = true;
    } else
    {
      //qDebug() << "iconv: " << myCodec;

      //in.setDevice(0);

      iconv_t cd = iconv_open("utf-16le", myCodec.toAscii().data());
      qDebug() << cd;
      // fail?
      if ((iconv_t)(-1) != cd)
      {
        size_t size = file.size();
        char *bufin = new char[size+8];
        memset(bufin, 0, size+8);
        char *bufout = new char[size*2+8];  // max. 2 symbols
        memset(bufout, 0, size*2+8);
        file.read(bufin, size);

  #ifdef Q_OS_WIN32
        const char *inptr = (const char *) bufin;
  #else
        char *inptr = bufin;
  #endif
        char *outptr = bufout;
        size_t newsize = size*2;
  //      qDebug() << (unsigned) newsize;
        //memset( &buf, 0, sizeof buf );
        errno = 0;
        iconv(cd, &inptr, &size, &outptr, &newsize);
        iconv_close(cd);
  //      qDebug() << (unsigned) size;
  //      qDebug() << (unsigned) newsize;
        int se = errno;

//      int sz = (outptr-bufout);
//      qDebug() << "converted: " << sz << "   error: " << se;

        delete[] bufin;

        if (se == 0) {
          QString txt = QString::fromUtf16((const ushort*)bufout);
          //qDebug() << txt;
          setText(txt);
          loaded = true;
        }

        delete[] bufout;
      }
    }
  }

  // if still not loaded - just load as is (and warn?)
  if (!loaded) {
    setText(in.readAll());
  }

  file.close();
  myChangeTime = QFileInfo(file).lastModified();
  //qDebug() << myChangeTime;
  setModified(false);
  return true;
}

bool EditWindow::save(const QString &name)
{
  QFile file(name.isEmpty() ? myFilename : name);
  if (!file.open(QFile::ReadWrite))
    return false;

  // check if strip spaces
  if (myGlobalConfig->trimOnSave)
    trimLastSpaces();

  // check if add new line to the end
  if (myGlobalConfig->addLineOnSave)
  {
    if (!isLineEmpty(lines()-1))
      append("\n");
  }

  // codec is in myCodec
  QTextStream out(&file);
  if (myCodec == "ANSI" || myCodec == "ASCII")
  {
    file.close();
    file.open(QFile::WriteOnly);
    out << text();
  } else
  {
    out.setCodec(myCodec.toAscii().data());
    if (out.codec()->name().toLower() != "system")
    {
      //qDebug() << "save codec: " << out.codec()->name();

      file.close();
      file.open(QFile::WriteOnly);
      out.setGenerateByteOrderMark(myBOM);
      out << text();
    } else
    { // write via iconv
      //qDebug() << "iconv: " << myCodec;

      iconv_t cd = iconv_open(myCodec.toAscii().data(), "utf-16le");
      //qDebug() << cd;
      // fail?
      if ((iconv_t)(-1) == cd)
        return false;

      file.close();
      out.setDevice(0);

      QString txt = text();
      size_t size = txt.size() * 2;

#ifdef Q_OS_WIN32
      const char *inptr = (char*) txt.utf16();
#else
      char *inptr = (char*) txt.utf16();
#endif

      char *buf = new char[size*3]; // define a lot
      char *outptr = buf;
      size_t newsize = size*2;
//      qDebug() << (unsigned) newsize;
      //memset( &buf, 0, sizeof buf );
      errno = 0;
      iconv(cd, &inptr, &size, &outptr, &newsize);
      iconv_close(cd);
//      qDebug() << (unsigned) size;
//      qDebug() << (unsigned) newsize;
      int se = errno;
      int sz = (outptr-buf);
      //qDebug() << "converted: " << sz << "   error: " << se;

      if (se != 0) {
        delete[] buf;
        return false;
      }

      file.open(QFile::WriteOnly);
      file.write(buf, sz);
      file.close();
      delete[] buf;
    }
  }

  file.close();
  myChangeTime = QFileInfo(file).lastModified();
  setModified(false);
  return true;
}

bool EditWindow::saveEncoded(const QString &codec)
{
  QString tmp = myCodec;

  if (codec.isEmpty())
    myCodec = "ANSI";
  else
    myCodec = codec;

  if (save())
    return true;

  myCodec = tmp;
  return false;
}

////////////////////////////////////////////////////////////////////////////////

void EditWindow::storeSettings(QSettings &/*settings*/)
{
}

void EditWindow::restoreSettings(QSettings &/*settings*/)
{
}
