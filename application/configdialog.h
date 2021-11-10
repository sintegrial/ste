#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QtGui>

namespace Ui {
    class ConfigDialog;
}

struct Config;

struct ConfigInit {
  QStringList *codecs;
  QList<QAction *> eols;
};

class ConfigDialog : public QDialog {
    Q_OBJECT
public:
    ConfigDialog(const ConfigInit &init, QWidget *parent = 0);
    ~ConfigDialog();

    int exec(Config *config);

signals:
    void configChanged();

protected slots:
    void discard();
    void apply();
    virtual void accept();

    void on_buttonBox_clicked ( QAbstractButton * button );

    void on_cbLexer_currentIndexChanged(int);
    void on_teCustomExt_textChanged();
    void on_teCustomWildcard_textChanged();

protected:
    void changeEvent(QEvent *e);

    void reloadCurrentLexer();

private:
    Ui::ConfigDialog *ui;

    Config *config;
};

#endif // CONFIGDIALOG_H
