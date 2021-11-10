#include "dslrender.h"
#include "dictview.h"

#include "Qsci/qsciscintilla.h"

DSLRender::DSLRender(QObject *parent) : AbstractDict(parent)
{
}

void DSLRender::createIndex()
{
  bool entry = false;

  for (int i = 0; i < myEditor->lines(); i++)
  {
    int pos = myEditor->SendScintilla(QsciScintilla::SCI_POSITIONFROMLINE, i);
    if (pos < 0)
      break;

    int ch = myEditor->SendScintilla(QsciScintilla::SCI_GETCHARAT, pos);

    if (ch == '#')
      continue;

    if (ch != '\r' && ch != '\n' && ch != '\t' && ch != ' ')
    {
      // skip if several one by one
      if (entry) continue;
      entry = true;
      myInfo->entries.append(i);
      continue;
    }

    entry = false;
  }
}

void DSLRender::render()
{
  int line = 0;
  if (myInfo->current >= 0)
    line = myInfo->entries.at(myInfo->current);
  else
    myInfo->current = 0;

  myInfo->text = "";
  myInfo->shown = 0;

  bool entry = false;

  for (int i = line; i < myEditor->lines(); i++)
  {
    QString txt = myEditor->text(i);

    if (txt.startsWith("#") || txt.isEmpty())
      continue;

    QChar ch = txt.at(0);
    if (ch != '\r' && ch != '\n' && ch != '\t' && ch != ' ')
    {
      if (!entry)
      {
        entry = true;

        if (myInfo->shown == myItemsOnPage)
          break;

        myInfo->shown++;

        if (myInfo->shown > 1 && myInfo->shown <= myItemsOnPage)
            myInfo->text += "<hr>";
      }
    } else
      entry = false;

    myInfo->text += renderString(txt);
  }
}

QString DSLRender::renderString(QString str)
{
  // check for header
  if (!str.startsWith(" ") && !str.startsWith("\t") && !str.trimmed().isEmpty())
  {
    str = "<h3>" + str + "</h3>";
    return str;
  }

   int pos;

   str.replace("[/m]", "");
   str.replace("[m0]", "");
   str.replace("[m1]", "&nbsp;");
   str.replace("[m2]", "&nbsp;&nbsp;");
   str.replace("[m3]", "&nbsp;&nbsp;&nbsp;");
   str.replace("[m4]", "&nbsp;&nbsp;&nbsp;&nbsp;");
   str.replace("[m5]", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
   str.replace("[m6]", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
   str.replace("[m7]", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
   str.replace("[m8]", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
   str.replace("[m9]", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");

   // remove/convert
   str.remove("[/m]");

   str.remove("[trn]");
   str.remove("[/trn]");

   str.remove("[!trs]");
   str.remove("[/!trs]");

   str.remove("[*]");
   str.remove("[/*]");

   str.replace("[b]", "<b>");
   str.replace("[/b]", "</b>");

   str.replace("[i]", "<i>");
   str.replace("[/i]", "</i>");

   str.replace("[u]", "<u>");
   str.replace("[/u]", "</u>");

   str.replace("[sub]", "<sub>");
   str.replace("[/sub]", "</sub>");

   str.replace("[sup]", "<sup>");
   str.replace("[/sup]", "</sup>");

   // ex tags
   str.replace("[ex]", exampleStart);
   str.replace("[/ex]", exampleEnd);

   // #### " " at the end is important!
   // ~
//   str.replace("~", QString("<b>%1</b>").arg(m_currentKeyPhrase));
   str.replace(">>>", "> >>");
   str.replace("<<<", "<< <");

   // << >> tags
   QRegExp rref("<<(.*)>>"); rref.setMinimal(true);
   //str.replace(rref, "<a href='\\1'>\\1</a>");
   pos = rref.indexIn(str);
   while (pos > -1) {
       str.replace(rref.cap(0), refStart(rref.cap(1)));
       pos = rref.indexIn(str, pos);
   }

   // ref tags
   QRegExp rref2("\\[ref\\](.*)\\[/ref\\]"); rref2.setMinimal(true);
   //str.replace(rref2, "<a href='\\1'>\\1</a>");
   pos = rref2.indexIn(str);
   while (pos > -1) {
       str.replace(rref2.cap(0), refStart(rref2.cap(1)));
       pos = rref2.indexIn(str, pos);
   }

   // url tags
   QRegExp rrefi("\\[url\\](.*)\\[/url\\]"); rrefi.setMinimal(true);
   pos = rrefi.indexIn(str);
   while (pos > -1) {
       str.replace(rrefi.cap(0), urlStart(rrefi.cap(1)));
       pos = rrefi.indexIn(str, pos);
   }

   // {{/n}} tags
   str.replace("{{/n}}", "<br>");

   // {{ ... }} tags - just remove
 QRegExp rbrack("\\{\\{.*\\}\\}"); rbrack.setMinimal(true);
 str.remove(rbrack);

   // lang tags
   QRegExp re("\\[lang.*\\]"); re.setMinimal(true);
   str.remove(re);
   str.remove("[/lang]");

   // c tags
   str.replace("[c]", highlightStart);

   // #### This does not work - bug in Qt?
   //QRegExp rc("\\[c\\s(.*)\\](.*)\\[/c\\]"); rc.setMinimal(true);
   //str.replace(rc, "<font color='\\1'>\\2</font>");

   // workaround...
   QRegExp rc("\\[c\\s(.*)\\]"); rc.setMinimal(true);
   str.replace(rc, "<font color='\\1'>");

   str.replace("[/c]", highlightEnd);

   // com tags
   str.replace("[com]", commentStart);
   str.replace("[/com]", commentEnd);

   // p tags
   //str.replace("[p]", "<u>");
   //str.replace("[/p]", "</u>");
   QRegExp rp("\\[p\\](.*)\\[/p\\]"); rp.setMinimal(true);
   pos = rp.indexIn(str);
   while (pos > -1) {
       str.replace(rp.cap(0), abbrStart(rp.cap(1)));
       pos = rp.indexIn(str, pos);
   }

   // '
   str.replace("[']", strongStart);
   str.replace("[/']", strongEnd);

   // []
   //str.replace("\\[", "<b><font color='#663333'>[ ");
   //str.replace("\\]", " ]</font></b>");

   // "\"
   str.replace("\\", "");

   // s tags
   QRegExp rs("\\[s\\](.*)\\[/s\\]"); rs.setMinimal(true);
   pos = rs.indexIn(str);
   while (pos > -1) {
       str.replace(rs.cap(0), multimediaStart(rs.cap(1)));
       pos = rs.indexIn(str, pos);
   }

   str += "<br>";
   return str;
}
