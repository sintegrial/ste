#include "searchbar.h"
#include "ui_searchbar.h"
#include "searchprogressdialog.h"
#include "utilsui.h"

#define ROLE_POSITION    Qt::UserRole + 1
#define ROLE_TIMES       Qt::UserRole + 2

SearchBar::SearchBar(QWidget *parent) :
    SideBarBase(parent),
    ui(new Ui::SearchBar)
{
    ui->setupUi(this);
    setObjectName("SearchBar");
    setupBar(tr("Search and Replace"), false);

    ui->twResults->header()->setResizeMode(1, QHeaderView::Fixed);
    ui->twResults->setColumnWidth(1, 100);

    ui->twResults->setTextElideMode(Qt::ElideLeft);

    connect(ui->tbFindNext, SIGNAL(pressed()), this, SIGNAL(next()));
    connect(ui->tbFindPrev, SIGNAL(pressed()), this, SIGNAL(prev()));
    connect(ui->tbSearchAll, SIGNAL(pressed()), this, SIGNAL(searchAll()));
    connect(ui->tbReplace, SIGNAL(pressed()), this, SIGNAL(replace()));
    connect(ui->tbReplaceAll, SIGNAL(pressed()), this, SIGNAL(replaceAll()));

    connect(ui->leFind->lineEdit(), SIGNAL(returnPressed()), ui->tbSearchAll, SLOT(click()));
    connect(ui->leFind->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(on_leFind_textChanged(QString)));
    connect(ui->leReplace->lineEdit(), SIGNAL(returnPressed()), ui->tbReplaceAll, SLOT(click()));

    QCompleter *comp1 = new QCompleter(this);
    //comp1->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    comp1->setCompletionMode(QCompleter::PopupCompletion);
  //    comp1->setWrapAround(true);
    ui->leFind->setCompleter(comp1);
    //comp1->setWidget(ui->leFind);

    QCompleter *comp2 = new QCompleter(this);
    //comp2->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    comp2->setCompletionMode(QCompleter::InlineCompletion);
  //    comp2->setWrapAround(true);
    //comp2->setWidget(ui->leReplace);
    ui->leReplace->setCompleter(comp2);

    domainMenu = new QMenu(this);
    actCurrent = domainMenu->addAction(tr("Whole Document"));
    actCurrent->setData(SD_FILE);
    actToCursor = domainMenu->addAction(tr("From the Beginning to Cursor"));
    actToCursor->setData(SD_TO_CURSOR);
    actFromCursor = domainMenu->addAction(tr("From Cursor to the End"));
    actFromCursor->setData(SD_FROM_CURSOR);
    actSelection = domainMenu->addAction(tr("Current Selection"));
    actSelection->setData(SD_SELECTION);
    actFiles = domainMenu->addAction(tr("Open Documents"));
    actFiles->setData(SD_OPENFILES);

    ui->tbDomain->setMenu(domainMenu);
    connect(domainMenu, SIGNAL(triggered(QAction*)), this, SLOT(domainChanged(QAction*)));

    ui->tbWrap->setChecked(true);

    myDialog = new SearchProgressDialog(this);

    domainChanged(actCurrent);
    on_leFind_textChanged("");
}

SearchBar::~SearchBar()
{
    delete ui;
}

void SearchBar::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SearchBar::setVisible ( bool visible )
{
  if (visible)
  {
    ui->leFind->lineEdit()->selectAll();
    ui->leFind->setFocus();
  }

  QWidget::setVisible(visible);
}

void SearchBar::setFileSearch()
{
  domainChanged(actFiles);
}

void SearchBar::setSearchText(const QString &text)
{
  clearMessage();

  // check domain
  if (text.isEmpty()) {
    if (domainMenu->defaultAction() == actSelection)
      domainChanged(actFromCursor);
  } else {
    if (text.contains("\n"))  // multiline
    {
      domainChanged(actSelection);
      //ui->leFind->setText("");
      myData.text += "."; // change text to update search
    } else {
      if (domainMenu->defaultAction() == actSelection)
        domainChanged(actCurrent);
      ui->leFind->lineEdit()->setText(text);
    }
  }

  ui->leFind->lineEdit()->selectAll();
  ui->leFind->setFocus();
}

QString SearchBar::searchText() const
{
  return ui->leFind->lineEdit()->text();
}

const SearchData& SearchBar::searchData()
{
  QString newText = ui->leFind->lineEdit()->text();
  myData.newsearch = (newText != myData.text);
  myData.text = newText;

  myData.replaceText = ui->leReplace->lineEdit()->text();

  myData.cs = ui->tbCase->isChecked();
  myData.re = ui->tbRegExp->isChecked();
  myData.wo = ui->tbWords->isChecked();
  myData.wrap = ui->tbWrap->isChecked();

  //qDebug() << "searchData(): domainMenu->activeAction(): " << domainMenu->defaultAction();
  SearchDomain newDom = (SearchDomain) domainMenu->defaultAction()->data().toInt();
  if (newDom != myData.dom) myData.newsearch = true;
  myData.dom = newDom;

  return myData;
}

void SearchBar::domainChanged(QAction *act)
{
  ui->tbDomain->setText(act->text());
  domainMenu->setDefaultAction(act);
  //qDebug() << "domainMenu->activeAction(): " << domainMenu->defaultAction();
}

void SearchBar::on_leFind_textChanged(const QString& text)
{
  bool on = text.length();
  ui->tbFindNext->setEnabled(on);
  ui->tbFindPrev->setEnabled(on);
  ui->tbSearchAll->setEnabled(on);
  ui->tbReplace->setEnabled(on);
  ui->tbReplaceAll->setEnabled(on);

  emit textChanged();
}

void SearchBar::showMessage(const QString &text)
{
  ui->resultLabel->setText(text);
  ui->leFind->setStyleSheet("");
}

void SearchBar::showMessageNotFound(const QString &text)
{
  ui->resultLabel->setText(text);
  ui->leFind->setStyleSheet("background-color: #ff9999;");
}

void SearchBar::clearMessage()
{
  ui->resultLabel->clear();
  ui->leFind->setStyleSheet("");
}

void SearchBar::clearResults()
{
  ui->twResults->clear();
}

void SearchBar::searchStarted(int count)
{
  fileMap.clear();
  ui->twResults->clear();

  ui->twResults->setUpdatesEnabled(false);
  ui->twResults->header()->setResizeMode(0, QHeaderView::Fixed);

  showMessage(tr("Search in progress..."));

  myDialog->start(count);

  QApplication::processEvents();
}

void SearchBar::searchProgress(int value)
{
  myDialog->setProgress(value);
}

void SearchBar::searchFinished()
{
  myDialog->hide();

  ui->twResults->header()->setResizeMode(0, QHeaderView::Stretch);

  // expand if not too much
  if (fileMap.count() == 1)
    ui->twResults->expandAll();

  // fill up count
  for (int i = 0; i < ui->twResults->topLevelItemCount(); i++)
  {
    QTreeWidgetItem *item = ui->twResults->topLevelItem(i);
    item->setText(1, tr("%1 found").arg(item->childCount()));
  }

  ui->twResults->setUpdatesEnabled(true);
}

void SearchBar::addResult(const QString &file, const QString &text, const SearchResult &res)
{
  QTreeWidgetItem *item = 0;
  if (fileMap.contains(file))
  {
    item = fileMap[file];
  }
  else
  {
    item = new QTreeWidgetItem(ui->twResults);
    ui->twResults->addTopLevelItem(item);
    fileMap[file] = item;

    item->setText(0, file);
    item->setBackgroundColor(0, Qt::lightGray);
    item->setBackgroundColor(1, Qt::lightGray);
  }

  QTreeWidgetItem *itemChild = new QTreeWidgetItem(item);

  // fake map to QRect : performance reasons
  itemChild->setData(0, ROLE_POSITION,
                     QRect(res.indexFrom, res.indexTo, res.lineFrom, res.lineTo));

  QString txt(text);
  txt.remove("\n"); txt.remove("\r");
  itemChild->setText(0, txt);

  itemChild->setText(1, QString("@ %1").arg(res.lineFrom+1));
}

void SearchBar::on_twResults_activated(const QModelIndex & index)
{
  if (!index.isValid())
    return;

  QList<QTreeWidgetItem *> list = ui->twResults->selectedItems();
  if (list.isEmpty())
    return;

  QTreeWidgetItem *item = list.first();
  if (ui->twResults->indexOfTopLevelItem(item) >= 0)
    return;

  SearchResult res;
  // fake map to QRect
  QRect r = item->data(0, ROLE_POSITION).toRect();
  res.indexFrom = r.x();
  res.indexTo = r.y();
  res.lineFrom = r.width();
  res.lineTo = r.height();

  emit resultClicked(item->parent()->text(0), item->text(0), res);
}



// config
void SearchBar::readConfig(QSettings &set)
{
  SideBarBase::readConfig(set);

  ui->tbCase->setChecked(set.value("searchCase", ui->tbCase->isChecked()).toBool());
  ui->tbWords->setChecked(set.value("searchWordsOnly", ui->tbWords->isChecked()).toBool());
  ui->tbWrap->setChecked(set.value("searchWrap", ui->tbWrap->isChecked()).toBool());
  ui->tbRegExp->setChecked(set.value("searchRegexp", ui->tbRegExp->isChecked()).toBool());

  UtilsUI::loadCombo(set, "searchCompleter", ui->leFind);
  UtilsUI::loadCombo(set, "replaceCompleter", ui->leReplace);
//  ui->leFind->blockSignals(true);
//  ui->leFind->lineEdit()->blockSignals(true);
//  ui->leReplace->blockSignals(true);
//  ui->leReplace->lineEdit()->blockSignals(true);
//
//  QStringList sl;
//  sl = set.value("searchCompleter").toStringList();
//  //ui->leFind->clear();
//  ui->leFind->addItems(sl);
//  QStringList rl;
//  rl = set.value("replaceCompleter").toStringList();
//  //ui->leReplace->clear();
//  ui->leReplace->addItems(rl);
//
//  ui->leFind->blockSignals(false);
//  ui->leFind->lineEdit()->blockSignals(false);
//  ui->leReplace->blockSignals(false);
//  ui->leReplace->lineEdit()->blockSignals(false);
}

void SearchBar::writeConfig(QSettings &set)
{
  SideBarBase::writeConfig(set);

  set.setValue("searchCase", ui->tbCase->isChecked());
  set.setValue("searchWordsOnly", ui->tbWords->isChecked());
  set.setValue("searchWrap", ui->tbWrap->isChecked());
  set.setValue("searchRegexp", ui->tbRegExp->isChecked());

  UtilsUI::storeCombo(set, "searchCompleter", ui->leFind);
  UtilsUI::storeCombo(set, "replaceCompleter", ui->leReplace);

//  QStringList sl;
//  for (int i = 0; i < ui->leFind->count(); i++)
//    sl.append(ui->leFind->itemText(i));
//  set.setValue("searchCompleter", sl);
//
//  sl.clear();
//  for (int i = 0; i < ui->leReplace->count(); i++)
//    sl.append(ui->leReplace->itemText(i));
//  set.setValue("replaceCompleter", sl);
}
