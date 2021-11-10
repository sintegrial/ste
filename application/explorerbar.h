#ifndef EXPLORERBAR_H
#define EXPLORERBAR_H

#include <QtGui>

namespace Ui {
    class ExplorerBar;
}

class ExplorerBar : public QWidget {
    Q_OBJECT
public:
    ExplorerBar(QWidget *parent = 0);
    ~ExplorerBar();

    void readConfig(QSettings &set);
    void writeConfig(QSettings &set);

    void navigate(const QString &name);
    void synchronizeIfNeeded(const QString &name);

signals:
    void fileClicked(const QString &name);
    void fileCurrent(const QString &name);

    void synchFileRequest();

protected slots:
    void on_tvDirs_activated(const QModelIndex & index);
    void leDir_returnPressed();
    void on_tbUp_clicked();
    void on_tbRoot_clicked();
    void doSelectFirst();

    void currentRowChanged(const QModelIndex & current, const QModelIndex & previous );
    void highlighted(const QModelIndex & current);

    void on_tbSynch_clicked();
    void on_tbPickItem_clicked();
    void on_leFilter_textChanged ( const QString & text );

    void on_actionOpen_triggered();
    void on_actionCopyPath_triggered();
    void on_actionCopyName_triggered();

protected:
    void changeEvent(QEvent *e);

private:
    void doSetPath(const QString & text, bool select = true);
    bool eventFilter(QObject *obj, QEvent *event);

    Ui::ExplorerBar *ui;

    QFileSystemModel *model;
};

#endif // EXPLORERBAR_H
