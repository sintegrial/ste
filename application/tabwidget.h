#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QtGui>

class TabWidget : public QTabWidget
{
Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0);
    virtual ~TabWidget() {}

    inline QTabBar * tabBar () const { return QTabWidget::tabBar(); }

signals:

protected slots:
    void tabChanged(int);
    void hoverTabMenu(QAction*);

protected:
    virtual void keyPressEvent ( QKeyEvent * event );

    bool eventFilter(QObject *obj, QEvent *event);

    QMenu *switchMenu;
    QWidget *lastTab;
};

#endif // TABWIDGET_H
