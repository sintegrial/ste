#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QtGui>

class ClickLabel : public QLabel
{
Q_OBJECT
public:
    explicit ClickLabel(QWidget *parent = 0);

    void setMenu(QMenu *menu) { myMenu = menu; }

signals:
    void pressed();
    void released();
    void clicked();
    void doubleClicked();

protected:
    void mousePressEvent ( QMouseEvent * ev );
    void mouseReleaseEvent ( QMouseEvent * ev );
    void mouseDoubleClickEvent( QMouseEvent * ev );

    QMenu *myMenu;
};

#endif // CLICKLABEL_H
