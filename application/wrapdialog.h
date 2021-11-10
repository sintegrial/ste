#ifndef WRAPDIALOG_H
#define WRAPDIALOG_H

#include <QtGui>

namespace Ui {
    class WrapDialog;
}

class WrapDialog : public QDialog {
    Q_OBJECT
public:
    WrapDialog(QWidget *parent = 0);
    ~WrapDialog();

    void readConfig(QSettings &set);
    void writeConfig(QSettings &set);

    virtual int exec();

    void getText(QString &s1, QString &s2);

protected slots:
    void text1changed(QString text);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::WrapDialog *ui;
};

#endif // WRAPDIALOG_H
