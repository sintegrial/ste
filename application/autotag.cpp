#include "mainwindow.h"
#include "editwindow.h"
#include "wrapdialog.h"

void MainWindow::autoTag()
{
  if (!textEdit) return;

  if (textEdit->hasSelectedText())
  {
    wrapSelected();
    return;
  }

  bool var = false;

  int cpos = textEdit->SendScintilla(SCI_GETCURRENTPOS);
  int line = textEdit->SendScintilla(SCI_LINEFROMPOSITION, cpos);
  int pos_start = textEdit->SendScintilla(SCI_POSITIONFROMLINE, line);
  int pos_end = textEdit->SendScintilla(SCI_GETLINEENDPOSITION, line);

  int c1 = 0, c2 = 0;
  int wstart = -1, wend = -1;

  // search for tag start
  int pos = cpos;
  while (pos >= pos_start)
  {
    //int c = textEdit->SendScintilla(SCI_GETCHARAT, pos);
    int c = textEdit->DirectCharAt(pos);
    //qDebug() << "pos " << pos << "  c " << c;

    if (c == '<' || c == '[') {
      c1 = c;
      wstart = pos+1;
      break;
    }

    if (c == '>' || c == ']')
    {
      // cursor is just after the last brace
      if (cpos-1 == pos)
      {
        c2 = c;
        wend = pos;
      } else // cursor is just before the last brace
        if (cpos == pos)
        {
          c2 = c;
          wend = pos;
        } else
          return;
    }

    pos--;
  }

  //qDebug() << "c1: " << c1 << " " << wstart;

  if (c1 == 0) return;

  // search for tag end
  if (c2 == 0)
  {
    pos = cpos+1;
    while (pos <= pos_end)
    {
      //int c = textEdit->SendScintilla(SCI_GETCHARAT, pos);
      int c = textEdit->DirectCharAt(pos);
      if (c == '<' || c == '[')
        return;
      if (c == '>' || c == ']')
      {
        c2 = c;
        wend = pos;
        break;
      }
      pos++;
    }
  }

  // c2 could be 0

  //qDebug() << "c2: " << c2 << " " << wend;

  // get tag name
  int tagend = textEdit->SendScintilla(SCI_WORDENDPOSITION, wstart, var);
  // if there's emptiness
  if (tagend == wstart)
    return;

  QString tag = textEdit->textFromRange(wstart, tagend);
  //qDebug() << "tag: " << tag;
  if (tag == "/")
    return;

  QString str;
  int insertPos = wend > 0 ? wend+1 : tagend;
  int cursorPos = wend > 0 ? wend+1 : tagend+1;

  // xml tag
  if (c1 == '<')
  {
    if (wend > 0)
    {
      str = "</" + tag + ">";
    }
    else
    {
      str = "></" + tag + ">";
    }
  }

  // dsl tag
  if (c1 == '[')
  {
    // fix closing tag if it ends with a number
    if (tag.at(tag.length()-1).isDigit())
      tag.chop(tag.length()-1);

    if (wend > 0)
    {
      str = "[/" + tag + "]";
    }
    else
    {
      str = "][/" + tag + "]";
    }
  }

  textEdit->beginUndoAction();
  textEdit->SendScintilla(SCI_INSERTTEXT, insertPos, str.toUtf8().data());
  textEdit->SendScintilla(SCI_SETCURRENTPOS, cursorPos);
  textEdit->SendScintilla(SCI_SETSEL, cursorPos, cursorPos);
  textEdit->endUndoAction();
}


void MainWindow::createWrapDialog()
{
  wrapDialog = new WrapDialog(this);
}

void MainWindow::wrapSelected()
{
  if (wrapDialog->exec() == QDialog::Rejected)
    return;

  QString s1, s2;
  wrapDialog->getText(s1, s2);

  int pos2 = textEdit->SendScintilla(SCI_GETSELECTIONEND);
  int pos1 = textEdit->SendScintilla(SCI_GETSELECTIONSTART);

  textEdit->beginUndoAction();

  // fill last first
  if (!s2.isEmpty())
  {
    textEdit->SendScintilla(SCI_INSERTTEXT, pos2, s2.toUtf8().data());
  }

  if (!s1.isEmpty())
  {
    textEdit->SendScintilla(SCI_INSERTTEXT, pos1, s1.toUtf8().data());
  }

  int cpos = textEdit->SendScintilla(SCI_GETCURRENTPOS);
  textEdit->SendScintilla(SCI_SETSEL, cpos, cpos);

  textEdit->endUndoAction();

  textEdit->setFocus();
}
