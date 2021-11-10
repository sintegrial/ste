#include "formatmanager.h"
#include "defines.h"
#include "toolmanager.h"

#include <iostream>
#include <strstream>

#include <uncrustify/src/prototypes.h>

////////////////////////////////////////////////////////////////////////////////

FormatManager::FormatManager(QObject *parent) : QObject(parent)
{
  // get formats
  formatPath = RESOURCE_DIR + "formats/";

  // init
  initAstyle();
  initUncrustify();
  initCSSTidy();
  initHTMLTidy();

  // register
  registerActions();
}

FormatManager::~FormatManager()
{
}

void FormatManager::registerActions()
{
  for (int i = 0; i < FT_COUNT; i++)
  {
    FormatterInfo &info = formatInfo[i];
    if (!info.enabled)
      continue;

    foreach(QAction* act, info.menu->actions())
    {
      actMap[act] = (FormatterType) i;
    }
  }
}

bool FormatManager::format(FormatterType type, QString &text, const QVariant &data)
{
  if (text.isEmpty())
    return false;

  switch (type)
  {
  case FT_ASTYLE:
    return formatAstyle(text, data.toString());
  case FT_UNCRUSTIFY:
    return formatUncrustify(text, data.toString());
  case FT_CSSTIDY:
    return formatCSSTidy(text, data.toString());
  case FT_HTMLTIDY:
    return formatHTMLTidy(text, data.toString());
  }
  return false;
}


bool FormatManager::writeFile(const QString &fileName, const QString &data)
{
  QFile fin(fileName);
  if (!fin.open(QFile::WriteOnly))
    return false;
  QTextStream tsi(&fin);
  tsi.setCodec("UTF-8");
  tsi << data;
  fin.close();
  return true;
}

bool FormatManager::readFile(const QString &fileName, QString &data)
{
  QFile fout(fileName);
  if (!fout.open(QFile::ReadOnly))
    return false;
  QTextStream ts(&fout);
  ts.setCodec("UTF-8");
  data = ts.readAll();
  return true;
}

int FormatManager::readFormatConfigs(FormatterInfo &info,
                                     const QString &dir,
                                     const QStringList &filter)
{
  QDir uncrDir(dir);
  QStringList uncrList = uncrDir.entryList(filter);
  for (int i = 0; i < uncrList.count(); i++)
  {
    QString name = uncrList.at(i);
    QString filename = dir + "/" + name;
    //uncrFormats.append(name);

    QFileInfo fi(filename);
    QAction *act = info.menu->addAction(fi.baseName());
    //act->setData(i);
    act->setData(name);

    //qDebug() << "   [" << name << "] from " << filename;
  }

  return uncrList.count();
}

////////////////////////////////////////////////////////////////////////////////

void FormatManager::initAstyle()
{
  FormatterInfo &info = formatInfo[FT_ASTYLE];

  info.menu = new QMenu(tr("Format via Artistic Style"));

  // check if can run astyle
#ifdef Q_WS_WIN
  info.path = QCoreApplication::applicationDirPath() + "/astyle.exe";
#else
  info.path = "astyle";
#endif

  info.enabled = (QProcess::execute(info.path, QStringList() << "--version") == 0);
  if (!info.enabled)
    return;

  QAction *formatCodeAct;
  formatCodeAct = info.menu->addAction(tr("Allman"));
  formatCodeAct->setData(/*STYLE_ALLMAN*/"allman");
  formatCodeAct = info.menu->addAction(tr("Java"));
  formatCodeAct->setData(/*STYLE_JAVA*/"java");
  formatCodeAct = info.menu->addAction(tr("K and R"));
  formatCodeAct->setData(/*STYLE_KandR*/"k&r");
  formatCodeAct = info.menu->addAction(tr("Stroustrup"));
  formatCodeAct->setData(/*STYLE_STROUSTRUP*/"stroustrup");
  formatCodeAct = info.menu->addAction(tr("Whitesmith"));
  formatCodeAct->setData(/*STYLE_WHITESMITH*/"whitesmith");
  formatCodeAct = info.menu->addAction(tr("Banner"));
  formatCodeAct->setData(/*STYLE_BANNER*/"banner");
  formatCodeAct = info.menu->addAction(tr("GNU"));
  formatCodeAct->setData(/*STYLE_GNU*/"gnu");
  formatCodeAct = info.menu->addAction(tr("Linux"));
  formatCodeAct->setData(/*STYLE_LINUX*/"linux");
  formatCodeAct = info.menu->addAction(tr("Horstmann"));
  formatCodeAct->setData(/*STYLE_HORSTMANN*/"horstmann");
  formatCodeAct = info.menu->addAction(tr("1TBS"));
  formatCodeAct->setData(/*STYLE_1TBS*/"1tbs");
}

bool FormatManager::formatAstyle(QString &in, QString lang)
{
  QString fname_in = QDir::tempPath() + "/astyle.in";
  QString fname_out = QDir::tempPath() + "/astyle.in";

  if (!writeFile(fname_in, in))
    return false;

  QStringList arg;
  arg << "--style=" + lang << fname_in;

  if (QProcess::execute(formatInfo[FT_ASTYLE].path, arg))
    return false;

  return readFile(fname_out, in);
}

////////////////////////////////////////////////////////////////////////////////

void uncrustify_file(const char *data, int data_len, FILE *pfout,
                            const char *parsed_file);

void FormatManager::initUncrustify()
{
  FormatterInfo &info = formatInfo[FT_UNCRUSTIFY];
  info.enabled = true;
  info.path = formatPath + "uncrustify";
  info.menu = new QMenu(tr("Format via Uncrustify"));

  //qDebug() << "Uncrustify configuration:";

  readFormatConfigs(info, info.path, QStringList() << "*.cfg");

  register_options();
}

bool FormatManager::formatUncrustify(QString &in, QString lang)
{
  set_option_defaults();

  QString cfg = formatInfo[FT_UNCRUSTIFY].path + "/" + lang;

  //qDebug() << "Uncrustify configuration loaded: [" << lang << "] from " << cfg;

  if (load_option_file(cfg.toAscii().data()))
    return false;

  // basic settings
  //cpd.settings[UO_newlines].le = (lineends_e)eoln;
  cpd.settings[UO_newlines].le = LE_CR;
  cpd.lang_flags = LANG_CPP;

  QByteArray ba = in.toUtf8();
  char *data = ba.data();

  QString fname = QDir::tempPath() + "/uncrustify.out";

  FILE *f = fopen(fname.toAscii().data(), "w");
  uncrustify_file(data, strlen(data), f, 0);
  fclose(f);

  return readFile(fname, in);
}

////////////////////////////////////////////////////////////////////////////////

void FormatManager::initCSSTidy()
{
  FormatterInfo &info = formatInfo[FT_CSSTIDY];

  info.menu = new QMenu(tr("Format via CSS Tidy"));

  // check if can run csstidy
#ifdef Q_WS_WIN
  info.path = QCoreApplication::applicationDirPath() + "/csstidy.exe";
#else
  info.path = "csstidy";
#endif

  info.enabled = (QProcess::execute(info.path, QStringList()) == 0);
  if (!info.enabled)
    return;

  //qDebug() << "CSS Tidy configuration:";

  readFormatConfigs(info, formatPath + "csstidy", QStringList() << "*.cfg");
}

bool FormatManager::formatCSSTidy(QString &in, QString lang)
{
  QString fname_in = QDir::tempPath() + "/csstidy.in";
  QString fname_out = QDir::tempPath() + "/csstidy.out";

  if (!writeFile(fname_in, in))
    return false;

  QStringList arg;
  arg << fname_in;

  QFile config(formatPath + "csstidy/" + lang);
  if (config.open(QFile::ReadOnly))
  {
    QTextStream ts(&config);
    arg << ts.readAll().split(" ", QString::SkipEmptyParts);
  }

  arg << fname_out;

  if (QProcess::execute(formatInfo[FT_CSSTIDY].path, arg))
    return false;

  return readFile(fname_out, in);
}

////////////////////////////////////////////////////////////////////////////////

void FormatManager::initHTMLTidy()
{
  FormatterInfo &info = formatInfo[FT_HTMLTIDY];

  info.menu = new QMenu(tr("Format via HTML Tidy"));

  // check if can run
#ifdef Q_WS_WIN
  info.path = QCoreApplication::applicationDirPath() + "/tidy.exe";
#else
  info.path = "tidy";
#endif

  info.enabled = (QProcess::execute(info.path, QStringList() << "--version") == 0);
  if (!info.enabled)
    return;

  //qDebug() << "HTML Tidy configuration:";

  readFormatConfigs(info, formatPath + "htmltidy", QStringList() << "*.cfg");
}

bool FormatManager::formatHTMLTidy(QString &in, QString lang)
{
  QString fname_in = QDir::tempPath() + "/htmltidy.in";
  QString fname_out = QDir::tempPath() + "/htmltidy.out";

  if (!writeFile(fname_in, in))
    return false;

  QString arg = formatInfo[FT_HTMLTIDY].path +
                " --char-encoding utf8" +
                " --doctype omit" +
                " --tidy-mark 0" +
                " --show-errors 0" +
                " --show-warnings 0" +
                " --quiet 1" +
                " --indent 1";

  QFile config(formatPath + "htmltidy/" + lang);
  if (config.open(QFile::ReadOnly))
  {
    QTextStream ts(&config);
    QString txt(ts.readAll());
    txt = txt.replace("\n", " ");
    arg += " " + txt;
  }

  arg += " --output-file \"" + fname_out + "\"" +
                 " \"" + fname_in + "\"";

  //qDebug() << arg;

  QProcess::execute(arg);

  return readFile(fname_out, in);
}
