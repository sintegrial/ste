#include "toolmanager.h"

#include <QDebug>

#include <uncrustify/src/uncrustify_version.h>
#include <enca.h>
#include <iconv.h>
#include <Qsci/qsciscintilla.h>

ToolManager::ToolManager(QObject *parent) :
    QObject(parent)
{
  ToolInfo info;
  QString res;

  // astyle
  res = getToolInfo("Artistic Style", "astyle", info, "--version");
  addToolInfo(info);

  // css tidy
  res = getToolInfo("CSS Tidy", "csstidy", info, "");
  addToolInfo(info);

  // html tidy
  res = getToolInfo("HTML Tidy", "tidy", info, "--version");
  addToolInfo(info);

  // uncrustify
  getBuiltinToolInfo("Uncrustify", info);
  info.version = UNCRUSTIFY_VERSION;
  addToolInfo(info);

  // enca
#ifdef Q_OS_WIN32
  getBuiltinToolInfo("enca", info);
#else
  res = getToolInfo("enca", "enca", info, "--version");
#endif
  addToolInfo(info);

  // iconv
#ifdef Q_OS_WIN32
  getBuiltinToolInfo("iconv", info);
  info.version = QString("%1.%2").arg(_LIBICONV_VERSION >> 8).arg(_LIBICONV_VERSION & 0xff);
#else
  res = getToolInfo("iconv", "iconv", info, "--version");
#endif
  addToolInfo(info);

  // qscintilla
  getBuiltinToolInfo("QScintilla", info);
  info.version = QSCINTILLA_VERSION_STR;
  addToolInfo(info);

  // qt
  getBuiltinToolInfo("Qt", info);
  info.version = QT_VERSION_STR;
  addToolInfo(info);
}

QString ToolManager::getToolInfo(const QString &name, const QString &exe, ToolInfo &info, QString params)
{
  QProcess process;
  process.setProcessChannelMode(QProcess::MergedChannels);

  info.name = name;
  info.version = "";
  info.builtin = false;

#ifdef Q_WS_WIN
  info.path = QCoreApplication::applicationDirPath() + "/" + exe + ".exe";
#else
  info.path = exe;
#endif

  process.start(info.path, params.split(" ", QString::SkipEmptyParts));
  info.present = (process.waitForFinished() && process.exitCode() == 0);
  if (info.present)
  {
    QString res = process.readAllStandardOutput();
    info.version = res.split("\n").first();
    return res;
  }

  return "";
}

void ToolManager::getBuiltinToolInfo(const QString &name, ToolInfo &info)
{
  info.path = tr("[built-in]");
  info.name = name;
  info.present = true;
  info.version = "";
  info.builtin = true;
}

void ToolManager::addToolInfo(ToolInfo &info)
{
  info.version.remove("\r");
  info.version.remove("\n");
  tools.append(info);
}
