#include "config.h"

#include <Qsci/qsciscintilla.h>

#include <QDebug>

void Config::readConfig(QSettings &set)
{
#ifdef Q_WS_WIN
  QString myFont("courier");
  int myEOL = QsciScintilla::EolWindows;
#endif
#ifdef Q_WS_X11
  QString myFont("monospace");
  int myEOL = QsciScintilla::EolUnix;
#endif
#ifdef Q_WS_MAC
  QString myFont("courier");
  int myEOL = QsciScintilla::EolMac;
#endif

  tabSize = set.value("tabSize", 4).toInt();
  spaceTabs = set.value("spaceTabs", false).toBool();
  indentSize = set.value("indentSize", 4).toInt();
  tabIndents = set.value("tabIndents", true).toBool();
  autoIndent = set.value("autoIndent", true).toBool();
  backspaceUnindent = set.value("backspaceUnindent", true).toBool();

  lineEdge = set.value("lineEdge", true).toBool();
  lineEdgeSize = set.value("lineEdgeSize", 80).toInt();

  fontSize = set.value("fontSize", 8).toInt();
  font = set.value("font", myFont).toString();
  colorScheme = set.value("colorScheme", "ste.stl").toString();

  foldStyle = set.value("foldStyle", 4 /*QsciScintilla::CircledTreeFoldStyle*/).toInt();

  autoCompletion = set.value("autoCompletion").toInt();
  autoCompletionCaseSensitive = set.value("autoCompletionCaseSensitive").toBool();
  autoCompletionReplaceWord = set.value("autoCompletionReplaceWord").toBool();

  insertPairs = set.value("insertPairs", true).toBool();

  defaultCodec = set.value("defaultCodec", "ASCII").toString();
  defaultEOL = set.value("defaultEOL", myEOL).toInt();
  defaultBOM = set.value("defaultBOM", true).toBool();

  trimOnSave = set.value("trimOnSave", true).toBool();
  addLineOnSave = set.value("addLineOnSave", true).toBool();

  language = set.value("language", "en").toString();
}

void Config::writeConfig(QSettings &set)
{
  set.setValue("tabSize", tabSize);
  set.setValue("indentSize", indentSize);
  set.setValue("spaceTabs", spaceTabs);
  set.setValue("autoIndent", autoIndent);
  set.setValue("backspaceUnindent", backspaceUnindent);
  set.setValue("tabIndents", tabIndents);

  set.setValue("lineEdgeSize", lineEdgeSize);
  set.setValue("lineEdge", lineEdge);

  set.setValue("fontSize", fontSize);
  set.setValue("font", font);
  set.setValue("colorScheme", colorScheme);

  set.setValue("foldStyle", foldStyle);

  set.setValue("autoCompletion", autoCompletion);
  set.setValue("autoCompletionCaseSensitive", autoCompletionCaseSensitive);
  set.setValue("autoCompletionReplaceWord", autoCompletionReplaceWord);

  set.setValue("insertPairs", insertPairs);

  set.setValue("defaultCodec", defaultCodec);
  set.setValue("defaultEOL", defaultEOL);
  set.setValue("defaultBOM", defaultBOM);

  set.setValue("trimOnSave", trimOnSave);
  set.setValue("addLineOnSave", addLineOnSave);

  set.setValue("language", language);
}
