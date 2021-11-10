#ifndef DICTVIEW_H
#define DICTVIEW_H

#include <QWidget>

struct DictViewInfo
{
  int current, shown;
  QList<int> entries;
  QString text;
};

namespace Ui {
    class DictView;
}

class DictView : public QWidget {
    Q_OBJECT
public:
    DictView(QWidget *parent = 0);
    ~DictView();

    void output(DictViewInfo *info);

public slots:
    virtual void setFocus();

signals:
    void prev();
    void next();
    void home();
    void end();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DictView *ui;
};

#endif // DICTVIEW_H
