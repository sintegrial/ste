#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

#define MAX_ZOOM  20
#define BASE_ZOOM  1

struct Config
{
  Config()
  {
  }

  void readConfig(QSettings &set);
  void writeConfig(QSettings &set);

  int tabSize;
  bool spaceTabs;

  int indentSize;
  bool autoIndent;
  bool backspaceUnindent;
  bool tabIndents;

  bool lineEdge;
  int lineEdgeSize;

  QString font;
  int fontSize;

  int foldStyle;

  int autoCompletion;
  bool autoCompletionCaseSensitive;
  bool autoCompletionReplaceWord;

  bool insertPairs;

  QString defaultCodec;
  int defaultEOL;
  bool defaultBOM;

  bool trimOnSave;
  bool addLineOnSave;

  QString colorScheme;

  QMap<int, QString> customExt, customWildcard;

  QString language;
};

#endif // CONFIG_H
