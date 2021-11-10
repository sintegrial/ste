#ifndef EDITDEFINES_H
#define EDITDEFINES_H

#include <QtCore>

struct TrimInfo
{
  int count;
  bool isAbove;
};


struct FormatInfo
{
  // words
  int count;    // 0 means dont change
  // space
  QString fill;
  // cells
  int cellWidth;
  bool cellExpand;
  Qt::Alignment cellAlign;
};


struct SearchFlags
{
  SearchFlags()
  {
    lineFrom = indexFrom = 0;
    lineTo = indexTo = 0;
    matchCase = false;
    backwards = false;
    isRegExp = false;
    wholeWords = false;
  }
  int lineFrom, indexFrom;
  int lineTo, indexTo;
  bool matchCase;
  bool backwards;
  bool isRegExp;
  bool wholeWords;
};

#endif // EDITDEFINES_H
