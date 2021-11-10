#ifndef SEARCHPROGRESSDIALOG_H
#define SEARCHPROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
    class SearchProgressDialog;
}

class SearchProgressDialog : public QDialog {
    Q_OBJECT
public:
    SearchProgressDialog(QWidget *parent = 0);
    ~SearchProgressDialog();

    void start(int count);
    void setProgress(int value);
    void hide();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SearchProgressDialog *ui;
};

#endif // SEARCHPROGRESSDIALOG_H
