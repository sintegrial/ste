#ifndef FILESDIALOG_H
#define FILESDIALOG_H

#include <QtGui>

namespace Ui {
    class FilesDialog;
}

class FilesDialog : public QDialog {
    Q_OBJECT
public:
    FilesDialog(QWidget *parent = 0);
    ~FilesDialog();

    void setText(const QString &header, const QString &top, const QString &bottom);
    void addButton(const QString &text, int res, bool def = false);
    void addItem(const QString &text);

    int itemCount() const;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::FilesDialog *ui;
    QButtonGroup *group;
};

#endif // FILESDIALOG_H
