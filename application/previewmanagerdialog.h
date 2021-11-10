#ifndef PREVIEWMANAGERDIALOG_H
#define PREVIEWMANAGERDIALOG_H

#include <QtGui>

namespace Ui {
    class PreviewManagerDialog;
}

class PreviewManagerDialog : public QDialog {
    Q_OBJECT
public:
    PreviewManagerDialog(QWidget *parent = 0);
    ~PreviewManagerDialog();

    void preview(QWidget *output);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::PreviewManagerDialog *ui;
};

#endif // PREVIEWMANAGERDIALOG_H
