#include "lexerdsl.h"

#include "editwindow.h"

#include <QDebug>

LexerDSL::LexerDSL(QObject *parent) : QsciLexerCustom(parent)
{
}

QString LexerDSL::description(int style) const
{
  switch (style)
  {
  case Default:
    return tr("Default");
  case Define:
    return tr("Define");
  case DefineValue:
    return tr("Defined Value");
  case Keyword:
    return tr("Keyword");
  case Entry:
    return tr("Entry");
  case Brace:
    return tr("Brace");
  case Error:
    return tr("Error");
  }

  return QString();
}

QColor LexerDSL::defaultColor(int style) const
{
  switch (style)
  {
  case Default:
    return Qt::black;
  case Define:
    return Qt::darkGray;
  case DefineValue:
    return Qt::darkGreen;
  case Keyword:
    return Qt::darkBlue;
  case Entry:
    return Qt::blue;
  case Brace:
    return Qt::darkRed;
  case Error:
    return Qt::red;
  }

  return QsciLexer::defaultColor(style);
}

void LexerDSL::styleText(int start, int end)
{
  if (!editor())
    return;

  int i, line, ch;

//  QTime t; t.start();
//  QTime lt; int lc = 0;

  while (start < end)
  {
    // skip empty lines
    while (start < end)
    {
      line = getLineAt(start);

      // cleanup folding
//      int v = 0;
//      editor()->SendScintilla(SCI_SETFOLDLEVEL, line, v);

      bool isEmtry = ((EditWindow*)editor())->isLineEmpty(line);
      if (isEmtry)
      {
        start = editor()->SendScintilla(SCI_POSITIONFROMLINE, line+1);
      }
      else
        break;
    }

    ch = getCharAt(start);

    // check for define
    if (ch == '#')
    {
      // color as define to the empty space
      i = start+1;
      while (i < end && isWordChar(getCharAt(i)))
        ++i;

      //startStyling(start);
      editor()->DirectStartStyling(start);
      //setStyling(i-start, Define);
      editor()->DirectSetStyle(i-start, Define);
      start = i;

      // color as define value to the end of the line
      i = getEOLfrom(line);
      startStyling(start);
      //setStyling(i-start, DefineValue);
      editor()->DirectSetStyle(i-start, DefineValue);

      // move to next
      start = editor()->SendScintilla(SCI_POSITIONFROMLINE, line+1);
      continue;
    }

    // check for entry
    if (isWordChar(ch))
    {
      // color as entry name to the end of the line
      i = getEOLfrom(line);
      //startStyling(start);
      editor()->DirectStartStyling(start);
      //setStyling(i-start, Entry);
      editor()->DirectSetStyle(i-start, Entry);

//      // folding
//      editor()->SendScintilla(
//          SCI_SETFOLDLEVEL,
//          line,
//          SC_FOLDLEVELBASE | SC_FOLDLEVELHEADERFLAG);

      // move to next line
      start = editor()->SendScintilla(SCI_POSITIONFROMLINE, line+1);
      continue;
    }

    // else, colorize dsl markup
    {
//      // folding
//      editor()->SendScintilla(
//          SCI_SETFOLDLEVEL,
//          line,
//          SC_FOLDLEVELBASE + 1);

      int eolpos = getEOLfrom(line);

      while (start < eolpos)
      {
        ch = getCharAt(start);

        if (ch == '[')
        {
          // check if it's a tag
          int pos = start;
          while (++pos < eolpos)
          {
            int ch1 = getCharAt(pos);

            if (ch1 == '[')
              break;

            if (ch1 == ']')
            {
              //lt.restart();

              // style brace
              //startStyling(start);
              editor()->DirectStartStyling(start);
              editor()->DirectSetStyle(1, Brace);
              //setStyling(1, Brace);

              // style tag
              //startStyling(start+1);
              editor()->DirectSetStyle(pos-start-1, Keyword);
              //setStyling(pos-start-1, Keyword);

              // style brace
              //startStyling(pos);
              editor()->DirectSetStyle(1, Brace);
              //setStyling(1, Brace);

              //lc += lt.elapsed();

              start = pos;
              break;
            }
          }

        } // if

        // inbetween tags
        start++;

      } // while

      // move to next line
      start = editor()->SendScintilla(SCI_POSITIONFROMLINE, line+1);

    } // DSL markup

  }

//  qDebug() << "dsl lexer time: " << t.elapsed();
//  qDebug() << "SCI_POSITIONFROMLINE time: " << lc;

}
