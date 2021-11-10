#include "abstractdict.h"
#include "dictview.h"

#include "Qsci/qsciscintilla.h"

QList<QByteArray> AbstractDict::imagesExt;
QList<QByteArray> AbstractDict::soundsExt;
//QMap<QString, QPixmap> AbstractDict::resourceMap;

AbstractDict::AbstractDict(QObject *parent) : QObject(parent)
{
  myInfo = new DictViewInfo;
  myItemsOnPage = 25;

  if (imagesExt.isEmpty()) {
        imagesExt = QImageReader::supportedImageFormats();
        //qDebug() << imagesExt;
    }

    if (soundsExt.isEmpty()) {
        soundsExt.append("wav");
    }
}

AbstractDict::~AbstractDict()
{
}

QWidget* AbstractDict::output(QsciScintilla *editor, const QString &baseDir)
{
  myEditor = editor;

  myInfo->current = -1;

  // parse and fill up entries
  int currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
  int currentLine = editor->SendScintilla(QsciScintilla::SCI_LINEFROMPOSITION, currentPos);

  // create line index
  createIndex();

  // check current line
  for (int i = 0; i < myInfo->entries.count(); i++)
  {
    int line = myInfo->entries.at(i);
    if (line == currentLine)
    {
      myInfo->current = i;
      break;
    }
    if (line > currentLine)
    {
      myInfo->current = i ? i-1 : 0;
      break;
    }
  }
  if (myInfo->current == -1)
    myInfo->current = myInfo->entries.count()-1;

  //qDebug() << "myInfo->entries(0) " << myInfo->entries.at(0);

  render();

  myView = new DictView(0);
  connect(myView, SIGNAL(home()), this, SLOT(home()));
  connect(myView, SIGNAL(end()), this, SLOT(end()));
  connect(myView, SIGNAL(prev()), this, SLOT(prev()));
  connect(myView, SIGNAL(next()), this, SLOT(next()));

  myView->output(myInfo);
  return myView;
}

void AbstractDict::prev()
{
  myInfo->current -= myItemsOnPage;
  render();
  myView->output(myInfo);
}

void AbstractDict::next()
{
  myInfo->current += myItemsOnPage;
  render();
  myView->output(myInfo);
}

void AbstractDict::home()
{
  myInfo->current = 0;
  render();
  myView->output(myInfo);
}

void AbstractDict::end()
{
  myInfo->current = myInfo->entries.count() - myItemsOnPage - 1;
  render();
  myView->output(myInfo);
}


const QString AbstractDict::styleSheet("<style>"
                "body { background-color: #FEFDEB; font-family: Verdana, Arial, Serif; } "
                "a { color: darkmagenta; } "
                "a:hover { color: magenta; } "
                "a.abbr { color: #008000; } "
                "a.noabbr { color: #800000; } "
                "a.url { color: darkblue; } "
                ".langs { color: #000080; font-style: italic; font-size: small; } "
                ".dicts { color: #000080; font-weight: bold; } "
                ".entry { background-color: lightgreen; border-width: 1px; border-color: green; border-style: dotted; } "
                "h3 { color: darkgreen; }"
                "</style>"
                );

const QString AbstractDict::exampleStart("&nbsp;&nbsp;&nbsp;&nbsp;<font color='#808080'>&#x2022; <i> ");
const QString AbstractDict::exampleEnd("</i></font>");

const QString AbstractDict::idiomStart("&nbsp;&nbsp;&nbsp;&nbsp;<font color='#80cc80'>&#x2022; <i> ");
const QString AbstractDict::idiomEnd("</i></font>");

const QString AbstractDict::transcriptionStart("<b>[ </b><font color='#008000'>");
const QString AbstractDict::transcriptionEnd("</font><b> ]</b>");

const QString AbstractDict::commentStart("<font color='#333333'>");
const QString AbstractDict::commentEnd("</font>");

const QString AbstractDict::highlightStart("<font color='#aaaacc'>");
const QString AbstractDict::highlightEnd("</font>");

const QString AbstractDict::strongStart("<font color='#cc3333'>");
const QString AbstractDict::strongEnd("</font>");

const QString AbstractDict::emphasisStart("<b>");
const QString AbstractDict::emphasisEnd("</b>");

const QString AbstractDict::phraseStart("<h3>");
const QString AbstractDict::phraseEnd("</h3><p>");

const QString AbstractDict::refStart(const QString &ref)
{
    QString rs(ref.simplified());
    QString href(rs);
    removeHtml(href);
    return QString("<a href='%1'>%2</a>").arg(href, rs);
}

const QString AbstractDict::urlStart(const QString &ref)
{
    QString rs(ref.simplified());
    QString href(rs);
    removeHtml(href);
    return QString("<a href='%1' class='url'>%2</a>").arg(href, rs);
}

const QString AbstractDict::multimediaStart(const QString &ref)
{
    QString mm = ref.simplified();
//    QByteArray ext = QFileInfo(mm).suffix().toLower().toAscii();
//    QString path = QFileInfo(m_dirPath + "/" + mm).canonicalFilePath();
//
//    if (imagesExt.contains(ext))
//    {
//        return QString("<img src='%1'>").arg(path);
//    }
//
//    if (soundsExt.contains(ext))
//    {
//        path.replace(":", "&x003a;");	// colons
//        return QString("<a href='wave-sound:%1'><img src=':/Resources/icon16_sound.png'></a>").arg(path);
//    }

    return QObject::tr("<b>[ multimedia element: %1 ]</b>").arg(mm);
}

const QString AbstractDict::abbrStart(const QString &abbr_src)
{
  return abbr_src;

//    QString abbr(abbr_src);
//    removeHtml(abbr);
//
//    if (m_abbr.isEmpty()) {
//        readAbbr();
//        // if still empty -> no abbrs
//        if (m_abbr.isEmpty())
//            m_abbr[" "] = " ";	// dummy
//    }
//
//    if (!m_abbr.contains(abbr))
//        //return QString("<b>[ Unknown abbreviation: %1 ]</b>").arg(abbr);
//        return QString("<a class='noabbr' href='%1@#$'>%2</a>")
//        .arg(QObject::tr("<b>Unknown abbreviation:</b><br>") + abbr, abbr);
//
//    QString val = m_abbr[abbr];
//    if (val.endsWith("@#$")) {
//        val = m_abbr[val.left(val.length()-3)];
//    }
//
//    return QString("<a class='abbr' href='%1@#$'>%2</a>")
//        .arg(QObject::tr("<b>Abbreviation:</b><br>") + val, abbr);
}

QString& AbstractDict::removeHtml(QString &str)
{
    static QRegExp r("(<.*>)");
    r.setMinimal(true);
    return str.remove(r);
}
