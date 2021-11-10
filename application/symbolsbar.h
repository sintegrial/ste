#ifndef SYMBOLSBAR_H
#define SYMBOLSBAR_H

#include <QtGui>

////////////////////////////////////////////////////////////////////////////////

class SymbolsModel : public QAbstractTableModel
{
public:
  SymbolsModel(QObject *parent);

  int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
  int columnCount ( const QModelIndex & parent = QModelIndex() ) const;

  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  void setUnicode(int u);

private:
  int _unicode;
};

////////////////////////////////////////////////////////////////////////////////

namespace Ui {
    class SymbolsBar;
}

class SymbolsBar : public QWidget {
    Q_OBJECT
public:
    SymbolsBar(QWidget *parent = 0);
    ~SymbolsBar();

    void readConfig(QSettings &set);
    void writeConfig(QSettings &set);

signals:
    void insertSymbol(const QString&);

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_sbSymbol_editingFinished();
    void on_tvSymbols_activated ( const QModelIndex & index );
    void on_tbUnicode_toggled(bool);

private:
    Ui::SymbolsBar *ui;
    SymbolsModel *myModel;
};

#endif // SYMBOLSBAR_H
