#include "utilsui.h"

void UtilsUI::storeCombo(QSettings &set, const QString &name, QComboBox *combo)
{
  QStringList sl;
  for (int i = 0; i < combo->count(); i++)
    sl.append(combo->itemText(i));
  set.setValue(name, sl);
}

void UtilsUI::loadCombo(QSettings &set, const QString &name, QComboBox *combo)
{
  combo->blockSignals(true);
  if (combo->lineEdit())
    combo->lineEdit()->blockSignals(true);

  QStringList sl;
  sl = set.value(name).toStringList();
  combo->clear();
  combo->addItems(sl);

  combo->blockSignals(false);
  if (combo->lineEdit())
    combo->lineEdit()->blockSignals(false);
}
