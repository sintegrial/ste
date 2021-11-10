#include "extlexerasm.h"

ExtLexerASM::ExtLexerASM(const QString &sid, QObject *parent) :
    QsciLexerASM(parent)
{
  myLang = sid.toAscii();
  myData = DATA[sid];
}

ExtLexerASM::~ExtLexerASM()
{
}

const char *ExtLexerASM::keywords(int set) const
{
  if (myData->keywords.contains(set))
    return myData->keywords[set].constData();

  return 0;
}

// statics
QMap<QString, ExtLexerASMData*> ExtLexerASM::DATA;

bool ExtLexerASM::registerData(const QString &sid, QMap<QString,QString> &map)
{
  ExtLexerASMData *data = new ExtLexerASMData();
  DATA[sid] = data;

  if (map.contains(".cpu_instruction"))
    data->keywords[1] = map[".cpu_instruction"].toAscii();

  if (map.contains(".fpu_instruction"))
    data->keywords[2] = map[".fpu_instruction"].toAscii();

  if (map.contains(".register"))
    data->keywords[3] = map[".register"].toAscii();

  if (map.contains(".directive"))
    data->keywords[4] = map[".directive"].toAscii();

  if (map.contains(".operand"))
    data->keywords[5] = map[".operand"].toAscii();

  if (map.contains(".ext_instruction"))
    data->keywords[6] = map[".ext_instruction"].toAscii();

  return true;
}
