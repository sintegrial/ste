#ifndef UTILSUI_H
#define UTILSUI_H

#include <QtGui>

class UtilsUI
{
public:
    static void storeCombo(QSettings &set, const QString &name, QComboBox *combo);
    static void loadCombo(QSettings &set, const QString &name, QComboBox *combo);
};

#endif // UTILSUI_H
