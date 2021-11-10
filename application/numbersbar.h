#ifndef NUMBERSBAR_H
#define NUMBERSBAR_H

#include "sidebarbase.h"

#include <QtGui>

namespace Ui {
    class NumbersBar;
}

class NumbersBar : public SideBarBase
{
    Q_OBJECT
public:
    NumbersBar(QWidget *parent = 0);
    ~NumbersBar();

    void setNumber(const QString &number);

signals:
    void getNumber(QString &number);
    void pasteNumber(const QString &number);

protected:
    void changeEvent(QEvent *e);

protected slots:
    void inputChanged(int) { setupNumber(10); }

    void decChanged(QString);
    void hexChanged(QString);
    void binChanged(QString);

    void on_tbOrder_clicked();

    void on_tbDecIn_clicked();
    void on_tbDecOut_clicked();
    void on_tbHexIn_clicked();
    void on_tbHexOut_clicked();
    void on_tbBinIn_clicked();
    void on_tbBinOut_clicked();

    void on_tvBin_clicked ( const QModelIndex & index );

private:
    void setupNumber(int sys);
    void block(bool);

    Ui::NumbersBar *ui;

    qulonglong mInput;
    bool mInputValid;
    int mBytes;
    class BinModel *model;
};

#endif // NUMBERSBAR_H
