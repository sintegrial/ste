#include "translationmanager.h"
#include "defines.h"

#include <QDebug>

TranslationManager *TranslationManager::instance_ = 0;

TranslationManager::TranslationManager(QObject *parent) :
    QObject(parent)
{
  instance_ = this;

  langPath = RESOURCE_DIR + "lang/";

  QDir langDir(langPath);
  QStringList langList = langDir.entryList(QStringList() << "*.qm");
  for (int i = 0; i < langList.count(); i++)
  {
    QFileInfo fi(langList.at(i));
    QString id = fi.completeSuffix().remove(".qm");
    langMap[id] = fi.baseName();
  }

  langMap["en"] = "English (Default)";
}

void TranslationManager::translate(const QString &lang)
{
  qtTranslator.load("qt_" + lang,
          QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  qApp->installTranslator(&qtTranslator);

  QDir langDir(langPath);
  QStringList langList = langDir.entryList(QStringList() << "*." + lang + ".qm");
  if (langList.isEmpty())
    return;

  myappTranslator.load(langList.first(), langPath);
  qApp->installTranslator(&myappTranslator);
}

QIcon TranslationManager::icon(const QString &lang)
{
  QPixmap ico(langPath + "flags/" + lang + ".png");
  return QIcon(ico);
}
