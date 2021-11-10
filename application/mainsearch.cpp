#include "mainwindow.h"
#include "editwindow.h"
#include "tabwidget.h"
#include "searchbar.h"

#define SEARCH_TRIM_MARGIN  50

void MainWindow::createSearchBar()
{
  searchBar = new SearchBar(this);
  addSideBar(searchBar);

  connect(searchBar, SIGNAL(next()), this, SLOT(searchNext()));
  connect(searchBar, SIGNAL(prev()), this, SLOT(searchPrev()));
  connect(searchBar, SIGNAL(searchAll()), this, SLOT(searchAll()));
  connect(searchBar, SIGNAL(textChanged()), this, SLOT(searchIncremental()));
  connect(searchBar, SIGNAL(replace()), this, SLOT(replace()));
  connect(searchBar, SIGNAL(replaceAll()), this, SLOT(replaceAll()));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchClosed()));
  connect(searchBar, SIGNAL(resultClicked(QString,QString,const SearchResult &)),
          this, SLOT(searchResult(QString,QString,const SearchResult &)));

  newSearch = true;
}

void MainWindow::searchClosed()
{
  for (int i = tabWidget->count()-1; i >= 0; i--)
  {
    EditWindow *te = (EditWindow*) tabWidget->widget(i);
    te->cleanIndicators(INDICATOR_SEARCH);
  }
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::search()
{
  showSideBar(searchBar);

  searchBar->setSearchText(textEdit->selectedText());

  // clean markup
  //textEdit->cleanIndicators(INDICATOR_SEARCH);
  //searchClosed();
}

void MainWindow::searchFiles()
{
  searchBar->setFileSearch();
  search();
}

void MainWindow::searchNext()
{
  doSearch(true);
}

void MainWindow::searchPrev()
{
  doSearch(false);
}

void MainWindow::searchAll()
{
  doSearchReplaceAll(false);
}

void MainWindow::searchIncremental()
{
  // clean markup
  textEdit->cleanIndicators(INDICATOR_SEARCH);

  SearchData data = searchBar->searchData();

  if (data.dom == SD_SELECTION)
    doSelectionSearch(true, false, data);
  else
    doDocumentSearch(true, false, data);
}

void MainWindow::doSearch(bool fw)
{
  // clean markup
  //textEdit->cleanIndicators(INDICATOR_SEARCH);
  //searchClosed();

  const SearchData& data = searchBar->searchData();

  if (!searchBar->isVisible() || data.text.isEmpty())
  {
    doSimpleSearch(fw, data);
    return;
  }

  switch (data.dom)
  {
  case SD_SELECTION:
    doSelectionSearch(fw, fw, data);
    return;

  default:
    doDocumentSearch(fw, fw, data);
    return;
  }
}

void MainWindow::doSimpleSearch(bool fw, SearchData data)
{
  int line = -1, index = -1;

  data.text = textEdit->selectedText();

  if (!fixSearchData(fw, &line, &index, data))
    return;

  // search from cursor position, wrapping, no regexp
  bool res = textEdit->findFirst(data.text,
                                 false,
                                 data.cs,
                                 data.wo,
                                 true,
                                 fw,
                                 line, index);

  showSearchResults(res);
}

void MainWindow::doDocumentSearch(bool fw, bool fix, struct SearchData data)
{
  int line = -1, index = -1;

  if (!fixSearchData(fix, &line, &index, data))
    return;

  // perform search
  bool res = textEdit->findFirst(data.text, data.re, data.cs, data.wo, data.wrap,
                                 fw, line, index);

  showSearchResults(res);
}

void MainWindow::doSelectionSearch(bool fw, bool fix, struct SearchData data)
{
  Selection sel = textEdit->currentSelection();
  int line = -1, index = -1;

  if (newSearch)
  {
    textEdit->baseSel = sel;
    if (fw)
    {
      line = sel.lineFrom;
      index = sel.indexFrom;
    } else
    {
      line = sel.lineTo;
      index = sel.indexTo;
    }
  }
  else
  {
    if (!fixSearchData(fix, &line, &index, data))
      return;
  }

//  qDebug() << line << " " << index << " " << sel.lineTo << " " << sel.indexTo;
//  qDebug() << data.text;

  // block
  textEdit->blockSignals(true);
  textEdit->setUpdatesEnabled(false);

  // dont wrap if selection domain
  bool res = textEdit->findFirst(data.text, data.re, data.cs, data.wo, false,
                                 fw, line, index);

  // check if out of selection
  if (res)
  {
    //qDebug() << "found";
    Selection lastSel = textEdit->currentSelection();
    if (fw) {
      if (lastSel.lineTo > textEdit->baseSel.lineTo ||
          (lastSel.lineTo == textEdit->baseSel.lineTo && lastSel.indexTo > textEdit->baseSel.indexTo))
        res = false;
    } else {
      if (lastSel.lineFrom < textEdit->baseSel.lineFrom ||
          (lastSel.lineFrom == textEdit->baseSel.lineFrom && lastSel.indexFrom < textEdit->baseSel.indexFrom))
        res = false;
    }
  }

  newSearch = !res;

  // highlight found text
  if (!res)
  {
    // return selection back
    textEdit->setCurrentSelection(textEdit->baseSel);

//
//    // markup
//    int start = textEdit->SendScintilla(SCI_GETSELECTIONSTART);
//    int end = textEdit->SendScintilla(SCI_GETSELECTIONEND);
//    textEdit->applyIndicator(start, end, INDICATOR_SEARCH);
  }

  textEdit->setUpdatesEnabled(true);
  textEdit->blockSignals(false);

  showSearchResults(res);
}

bool MainWindow::fixSearchData(bool fw, int *line, int *index, SearchData &data)
{
  if (data.text.isEmpty())
  {
    statusBar()->showMessage(tr("Nothing to search."));
    newSearch = true;
    return false;
  }

  // check forward search
  if (!fw && data.text.length())
  {
    int pos = textEdit->SendScintilla(SCI_GETCURRENTPOS);
    pos -= data.text.toUtf8().length();
    textEdit->lineIndexFromPosition(pos, line, index);
  }

  return true;
}

void MainWindow::showSearchResults(bool res)
{
  if (!res)
  {
    searchBar->showMessageNotFound(tr("Search string not found"));
    newSearch = true;
  }
  else
  {
    searchBar->clearMessage();
    newSearch = false;
  }
}

void MainWindow::selectionChanged()
{
  if (textEdit->hasSelectedText())
  {
    //statusBar()->showMessage("Selection!", 2000);
    newSearch = true;
  };

  if (textEdit->selectedText().size() > 1)
  {
    // apply quick markup
    textEdit->applyIndicator(textEdit->selectedText(), INDICATOR_QUICK_SEARCH1, INDICATOR_QUICK_SEARCH2);
  } else
  {
    // remove quick markup
    textEdit->cleanIndicators(INDICATOR_QUICK_SEARCH1, INDICATOR_QUICK_SEARCH2);
  }
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::doSearchReplaceAll(bool replace)
{
  //qDebug() << "doSearchReplaceAll()";

  // clean indicators
  searchClosed();

  // clean results
  searchBar->clearResults();

  const SearchData &data = searchBar->searchData();
  if (data.text.isEmpty())
  {
    statusBar()->showMessage(tr("Nothing to search."));
    return;
  }

  // start search
  int count = 0;

  QTime time;
  time.start();

  //setEnabled(false);

  // check if file search
  if (data.dom == SD_OPENFILES)
  {
    searchBar->searchStarted();
    count = doOpenFilesSearch(data, replace);
  }
  else
  {
    searchBar->searchStarted(textEdit->length());
    count = doFileSearch(textEdit, data, replace);
  }

  // finish search
  int sec = time.elapsed() / 1000;

  searchBar->searchFinished();
//  setEnabled(true);

  if (!count)
    showSearchResults(false);
  else {
    if (replace)
      searchBar->showMessage(tr("%1 entries replaced [%2 s]").arg(count).arg(sec));
    else
      searchBar->showMessage(tr("%1 entries found [%2 s]").arg(count).arg(sec));

    newSearch = true;
  }
}

int MainWindow::doFileSearch(EditWindow *edit, struct SearchData data, bool replace)
{
  TextToFind ttf;

  int count = 0;

  int last;
  if (newSearch)
  {
    edit->baseSel = edit->currentSelection();
  }

  // clean markup
  edit->cleanIndicators(INDICATOR_SEARCH);

  // check domain
  switch (data.dom)
  {
  case SD_FILE:
    ttf.chrg.cpMin = 0;
    last = ttf.chrg.cpMax = edit->length();
    break;

  case SD_FROM_CURSOR:
    ttf.chrg.cpMin = edit->SendScintilla(SCI_GETCURRENTPOS);
    last = ttf.chrg.cpMax = edit->length();
    //qDebug() << "from cursor " << ttf.chrg.cpMin << "-" << last;
    break;

  case SD_TO_CURSOR:
    //qDebug() << "to cursor";
    ttf.chrg.cpMin = 0;
    last = ttf.chrg.cpMax = edit->SendScintilla(SCI_GETCURRENTPOS);
    break;

  case SD_SELECTION:
    // return selection back
    edit->setCurrentSelection(edit->baseSel);
    ttf.chrg.cpMin = edit->SendScintilla(SCI_GETSELECTIONSTART);
    last = ttf.chrg.cpMax = edit->SendScintilla(SCI_GETSELECTIONEND);
    break;

  default:
    return 0;
  }

  // search
  int flags = 0;
  if (data.cs) flags |= SCFIND_MATCHCASE;
  if (data.wo) flags |= SCFIND_WHOLEWORD;
  if (data.re) flags |= SCFIND_REGEXP;

  //searchBar->searchStarted();
  edit->setUpdatesEnabled(false);
  edit->beginUndoAction();

  int res, end;
  SearchResult sres;
  QByteArray ba = data.text.toUtf8();
  QByteArray ra = data.replaceText.toUtf8();
  do {
    ttf.lpstrText = ba.data();
    res = edit->SendScintilla(SCI_FINDTEXT, flags, &ttf);
    if (res < 0)
      break;

    if (ttf.chrgText.cpMax > last)
      break;

    count++;

    if (replace)
    {
      edit->SendScintilla(SCI_SETTARGETSTART, ttf.chrgText.cpMin);
      edit->SendScintilla(SCI_SETTARGETEND, ttf.chrgText.cpMax);

      edit->SendScintilla(SCI_REPLACETARGET, -1, ra.data());

      // target set to new
      end = edit->SendScintilla(SCI_GETTARGETEND);
      ttf.chrg.cpMin = end;
      ttf.chrg.cpMax = last;  // can be shorter or longer than initial
    }
    else
    {
      ttf.chrg.cpMin = ttf.chrgText.cpMax+1;
      end = ttf.chrgText.cpMax;
    }

    // markup
    edit->applyIndicator(ttf.chrgText.cpMin, end, INDICATOR_SEARCH);

    // results
    edit->lineIndexFromPosition(ttf.chrgText.cpMin, &sres.lineFrom, &sres.indexFrom);
    edit->lineIndexFromPosition(end, &sres.lineTo, &sres.indexTo);

    // trim search line
    QString resText(edit->text(sres.lineFrom));
    if (sres.indexFrom > SEARCH_TRIM_MARGIN)
      resText = "..." + resText.mid(sres.indexFrom-SEARCH_TRIM_MARGIN);
    if (sres.indexTo < resText.length()-SEARCH_TRIM_MARGIN)
      resText = resText.left(sres.indexTo+SEARCH_TRIM_MARGIN) + "...";

    searchBar->addResult(edit->fileName(), resText, sres);

    if (count % 10 == 0)
      searchBar->searchProgress(ttf.chrgText.cpMin);

  } while (res >= 0);

  edit->endUndoAction();
  edit->setUpdatesEnabled(true);
  //searchBar->searchFinished();

  return count;
}

int MainWindow::doOpenFilesSearch(struct SearchData data, bool replace)
{
  int count = 0;

  data.dom = SD_FILE;
  data.wrap = false;

  for (int i = 0; i < tabWidget->count(); i++)
  {
    EditWindow *te = (EditWindow*) tabWidget->widget(i);
    count += doFileSearch(te, data, replace);
  }

  return count;
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::replace()
{
  const SearchData &data = searchBar->searchData();
  textEdit->replace(data.replaceText);
}

void MainWindow::replaceAll()
{
  doSearchReplaceAll(true);
}

////////////////////////////////////////////////////////////////////////////////

void MainWindow::searchResult(const QString &fileName, const QString &/*text*/, const SearchResult &res)
{
  // check if such a file opened already
  EditWindow *te = findTab(fileName);
  if (te)
  {
    // switch to this tab
    tabWidget->setCurrentWidget(te);
  }
  else
  {
    // open the file, it will switch automatically
    loadFile(fileName);
  }

  if (res.lineTo >= textEdit->lines())
    return;

  textEdit->setCursorPosition(res.lineFrom, res.indexFrom);
  // select text if equal to the pattern
  //if (text == textEdit->text(sres.lineFrom))
  textEdit->setSelection(res.lineFrom, res.indexFrom, res.lineTo, res.indexTo);
  textEdit->ensureLineVisible(res.lineFrom);
  textEdit->ensureCursorVisible();
}
