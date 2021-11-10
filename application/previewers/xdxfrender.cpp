#include "xdxfrender.h"
#include "dictview.h"

#include "Qsci/qsciscintilla.h"

XDXFRender::XDXFRender(QObject *parent) : AbstractDict(parent)
{
}

void XDXFRender::createIndex()
{
  for (int i = 0; i < myEditor->lines(); i++)
  {
    int pos = myEditor->SendScintilla(QsciScintilla::SCI_POSITIONFROMLINE, i);
    if (pos < 0)
      break;

    int ch1 = myEditor->DirectCharAt(pos);
    int ch2 = myEditor->DirectCharAt(pos+1);
    int ch3 = myEditor->DirectCharAt(pos+2);
    int ch4 = myEditor->DirectCharAt(pos+3);

    if (ch1 == '<' && ch2 == 'a' && ch3 == 'r' && ch4 == '>')
    {
      // skip if several one by one
      myInfo->entries.append(i);
      continue;
    }
  }
}

void XDXFRender::render()
{
  int line = 0;
  if (myInfo->current >= 0)
    line = myInfo->entries.at(myInfo->current);
  else
    myInfo->current = 0;

  myInfo->text = "";
  myInfo->shown = 0;

  for (int i = line; i < myEditor->lines(); i++)
  {
    QString txt = myEditor->text(i).simplified();
    if (txt.isEmpty())
      continue;

    if (txt.startsWith("<ar>"))
    {
      if (myInfo->shown == myItemsOnPage)
        break;

      myInfo->shown++;

      if (myInfo->shown > 1 && myInfo->shown <= myItemsOnPage)
          myInfo->text += "<hr>";
    }

    myInfo->text += renderString(txt);
  }
}

QString XDXFRender::renderString(QString str)
{
  int pos;

  str.remove(QRegExp("<.?ar>"));
  str.remove(QRegExp("<.?nu>"));
  str.remove(QRegExp("<.?opt>"));

  str.replace("<k>", phraseStart);
  str.replace("</k>", phraseEnd);

  str.replace("<dtrn>", highlightStart);	// #### revise
  str.replace("</dtrn>", highlightEnd);

  str.replace("<tr>", transcriptionStart);
  str.replace("</tr>", transcriptionEnd);

  // ex tags
  str.replace("<ex>", exampleStart);
  str.replace("</ex>", exampleEnd);

  // abr tags
  QRegExp rp("<abr>(.*)</abr>"); rp.setMinimal(true);
  pos = rp.indexIn(str);
  while (pos > -1) {
      str.replace(rp.cap(0), abbrStart(rp.cap(1)));
      pos = rp.indexIn(str);
  }

  // co tags
  str.replace("<co>", commentStart);
  str.replace("</co>", commentEnd);

  // kref tags
  QRegExp rrefk("<kref>(.*)</kref>"); rrefk.setMinimal(true);
  pos = rrefk.indexIn(str);
  while (pos > -1) {
      str.replace(rrefk.cap(0), refStart(rrefk.cap(1)));
      pos = rrefk.indexIn(str);
  }

  // iref tags
  QRegExp rrefi("<iref>(.*)</iref>"); rrefi.setMinimal(true);
  pos = rrefi.indexIn(str);
  while (pos > -1) {
      str.replace(rrefi.cap(0), urlStart(rrefi.cap(1)));
      pos = rrefi.indexIn(str);
  }

  // rref tags
  QRegExp rref("<rref>(.*)</rref>"); rref.setMinimal(true);
  pos = rref.indexIn(str);
  while (pos > -1) {
      str.replace(rref.cap(0), multimediaStart(rref.cap(1)));
      pos = rref.indexIn(str);
  }

  // c tags
  str.replace("<c>", highlightStart);

  QRegExp rc("<c\\sc=(.*)>"); rc.setMinimal(true);
  str.replace(rc, "<font color=\\1>");

  str.replace("</c>", highlightEnd);

  // def tags
  str.replace("<def>", "<br>* ");

  str += "<br>";
  return str;
}
