#ifndef SHORTCUTBAR_H
#define SHORTCUTBAR_H

#include "sidebarbase.h"

namespace Ui {
    class ShortcutBar;
}

class ShortcutBar : public SideBarBase {
    Q_OBJECT
public:
    ShortcutBar(QWidget *parent = 0);
    ~ShortcutBar();

    void addItem(QAction *act);
    void addItems(QList<QAction*> &items);
    void addItems(QMenu *menu);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ShortcutBar *ui;
};

#endif // SHORTCUTBAR_H
