#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include "sidebarbase.h"
#include <QMenu>

enum SearchDomain { SD_FILE, SD_SELECTION, SD_TO_CURSOR, SD_FROM_CURSOR, SD_OPENFILES };

struct SearchData
{
  SearchData() : dom(SD_FILE), newsearch(true)
  {
  }

  QString text;
  QString replaceText;

  bool re;
  bool cs;
  bool wo;
  bool wrap;

  SearchDomain dom;
  bool newsearch;
};

struct SearchResult
{
  bool isCase;
  int lineFrom, indexFrom;
  int lineTo, indexTo;
  int idxFrom, idxTo;
};

typedef QList<SearchResult> SearchResults;


namespace Ui {
    class SearchBar;
}

class SearchBar : public SideBarBase {
    Q_OBJECT
public:
    SearchBar(QWidget *parent = 0);
    virtual ~SearchBar();

    void searchStarted(int count = 0);
    void searchProgress(int value);
    void searchFinished();

    // config
    void readConfig(QSettings &set);
    void writeConfig(QSettings &set);

public slots:
    virtual void setVisible ( bool visible );

    void setSearchText(const QString &text);
    QString searchText() const;
    const SearchData& searchData();

    void setFileSearch();

    void showMessage(const QString &text);
    void showMessageNotFound(const QString &text);
    void clearMessage();

    void clearResults();
    void addResult(const QString &file, const QString &text, const SearchResult &res);

signals:
    void next();
    void prev();
    void searchAll();
    void textChanged();

    void replace();
    void replaceAll();

    void resultClicked(const QString &file, const QString &text, const SearchResult &res);

protected:
    void changeEvent(QEvent *e);

protected slots:
    void domainChanged(QAction *act);
    void on_leFind_textChanged(const QString&);
    void on_twResults_activated(const QModelIndex & index);

private:
    Ui::SearchBar *ui;

    SearchData myData;

    QMenu *domainMenu;
    QAction *actCurrent, *actFromCursor, *actToCursor, *actSelection, *actFiles;

    QMap<QString, QTreeWidgetItem*> fileMap;

    class SearchProgressDialog *myDialog;
};

#endif // SEARCHBAR_H
