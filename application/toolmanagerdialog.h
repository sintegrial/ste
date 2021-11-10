#ifndef TOOLMANAGERDIALOG_H
#define TOOLMANAGERDIALOG_H

#include <QDialog>

namespace Ui {
    class ToolManagerDialog;
}

class ToolManagerDialog : public QDialog {
    Q_OBJECT
public:
    ToolManagerDialog(QWidget *parent, class ToolManager *manager);
    ~ToolManagerDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ToolManagerDialog *ui;
};

#endif // TOOLMANAGERDIALOG_H
