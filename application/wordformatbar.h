#ifndef WORDFORMARBAR_H
#define WORDFORMARBAR_H

#include "sidebarbase.h"
#include "editdefines.h"

namespace Ui {
    class WordFormatBar;
}

class WordFormatBar : public SideBarBase
{
    Q_OBJECT
public:
    WordFormatBar(QWidget *parent = 0);
    ~WordFormatBar();

signals:
    void format(FormatInfo &info);

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_pbFormat_clicked();

private:
    Ui::WordFormatBar *ui;
};

#endif // WORDFORMARBAR_H
