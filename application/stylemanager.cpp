#include "stylemanager.h"
#include "defines.h"

#include <lexers/extlexerasm.h>
#include <lexers/lexerdsl.h>

#include <SciLexer.h>

#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexerbatch.h>
#include <Qsci/qscilexercmake.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercsharp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerd.h>
#include <Qsci/qscilexerdiff.h>
#include <Qsci/qscilexerfortran.h>
#include <Qsci/qscilexerfortran77.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexeridl.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexermakefile.h>
#include <Qsci/qscilexerpascal.h>
#include <Qsci/qscilexerperl.h>
#include <Qsci/qscilexerpostscript.h>
#include <Qsci/qscilexerpov.h>
#include <Qsci/qscilexerproperties.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerruby.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexertcl.h>
#include <Qsci/qscilexertex.h>
#include <Qsci/qscilexervhdl.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexeryaml.h>

#if QSCINTILLA_VERSION >= 0x020401
#include <Qsci/qscilexerspice.h>
#include <Qsci/qscilexerverilog.h>
#endif

// Sintegrial lexers
#include <Qsci/qscilexerada.h>
#include <Qsci/qscilexerasm.h>
#include <Qsci/qscilexernsis.h>
#include <Qsci/qscilexerlisp.h>
#include <Qsci/qscilexerhaskell.h>

////////////////////////////////////////////////////////////////////////////////

const CommentInfo CommentASM(";");
const CommentInfo CommentBasic("rem ");
const CommentInfo CommentFortran("!");
const CommentInfo CommentC("//", "/*", "*/");
const CommentInfo CommentCS("///");
const CommentInfo CommentXML("", "<!--", "-->");
const CommentInfo CommentPAS("", "{", "}");
const CommentInfo CommentPS("@");
const CommentInfo CommentCmake("#");
const CommentInfo CommentSQL("--");
const CommentInfo CommentTEX("%");
const CommentInfo CommentLUA("--", "--[[", "]]--");
const CommentInfo CommentYAML("---");
const CommentInfo CommentSpice("*");

////////////////////////////////////////////////////////////////////////////////

StyleManager * StyleManager::This = 0;

StyleManager::StyleManager(QWidget *parent) : QObject(parent)
{
  This = this;

  stylesMenu = 0;
  group = new QActionGroup(this);
  group->setExclusive(true);

  initAPIs();
  initStyles();

  // default style
  //loadCurrentStyle("ste.stl");
}

StyleManager::~StyleManager()
{
  qDeleteAll(apis);
}

////////////////////////////////////////////////////////////////////////////////

void StyleManager::setMenu(QMenu *menu)
{
  stylesMenu = menu;
  connect(stylesMenu, SIGNAL(triggered(QAction*)), this, SLOT(setLexer(QAction*)));

  // fill up
  QAction *act = menu->addAction(tr("No style"));
  act->setCheckable(true);
  act->setData(NONE);
  group->addAction(act);

  LexerInfo info;
  info.act = act;
  info.name = "";
  LexerActions[NONE] = info;

  menu->addSeparator();

  // add built-in lexers
  addLexer("ada", "Ada", "ada|ads|adb", "", CommentSQL);
  addLexer("asm", "Assembler x86", "asm|inc", "", CommentASM);
  addLexer("asp", "ASP", "asp", "", CommentC);
  addLexer("bash", "Bash Script", "sh|bsh", "", CommentCmake);
  addLexer("batch", "Windows Batch Script", "bat|cmd|nt", "", CommentBasic);
  addLexer("cmake", "CMake", "cmake", "", CommentCmake);
  addLexer("cplusplus", "C and C++", "h|hh|hpp|h++|hxx|c|cc|cpp|c++|cxx", "", CommentC);
  addLexer("csharp", "C#", "cs", "", CommentCS);
  addLexer("css", "CSS", "css|qss|map|stl");
  addLexer("d", "D", "d", "", CommentC);
  addLexer("diff", "Diff", "diff|patch");
  addLexer("dsl", "DSL Dictionary", "dsl");
  addLexer("fortran", "Fortran", "f|for|f90|f95|f2k", "", CommentFortran);
  addLexer("fortran77", "Fortran 77", "f77", "", CommentFortran);
  addLexer("haskell", "Haskell", "hs|lhs|as|las", "", CommentSQL);
  addLexer("html", "HTML", "html|htm|shtml|shtm|dhtml|dhtm|xhtml", "", CommentXML);
  addLexer("idl", "IDL", "idl|midl", "", CommentC);
  addLexer("java", "Java", "java|jsp", "", CommentC);
  addLexer("js", "JavaScript", "js", "", CommentC);
  addLexer("lisp", "Lisp", "lsp|lisp", "", CommentASM);
  addLexer("lua", "Lua", "lua", "", CommentLUA);
  addLexer("makefile", "Makefile", "makefile|mak", "makefile*.*", CommentCmake);
  addLexer("nsis", "NSIS Script", "nsi|nsh", "", CommentASM);
  addLexer("pascal", "Pascal", "pas|inc|dpr", "", CommentPAS);
  addLexer("perl", "Perl", "pl|perl|pm|plx", "", CommentCmake);
  addLexer("php", "PHP", "php|php3|php4|php4|phtm|phtml", "", CommentC);
  addLexer("ps", "PostScript", "ps", "", CommentPS);
  addLexer("pov", "POV-Ray SDL", "pov", "", CommentC);
  addLexer("props", "Properties", "ini|inf|reg|url", "", CommentASM);
  addLexer("python", "Python", "py|pyw", "", CommentCmake);
  addLexer("ruby", "Ruby", "ruby|rb|rbw", "", CommentCmake);
  addLexer("sgml", "SGML", "sgml", "", CommentXML);
#if QSCINTILLA_VERSION >= 0x020401
  addLexer("spice", "Spice", "spice|mod", "", CommentSpice);
#endif
  addLexer("sql", "SQL", "sql", "", CommentSQL);
  addLexer("tcl", "TCL", "tcl", "", CommentCmake);
  addLexer("tex", "TeX", "tex|latex", "", CommentTEX);
  addLexer("vb", "Visual Basic", "vb|vbs", "", CommentBasic);
#if QSCINTILLA_VERSION >= 0x020401
  addLexer("verilog", "Verilog", "v|vl|verilog", "", CommentC);
#endif
  addLexer("vhdl", "VHDL", "vhd|vhdl", "", CommentSQL);
  addLexer("xml", "XML",
           "xml|mxml|vrml|xsml|xsl|xsd|kml|wsdl|ui|ts|vcproj|vcxproj",
           "*.kdev*", CommentXML);
  addLexer("xdxf", "XDXF", "xdxf", "", CommentXML);
  addLexer("yaml", "YAML", "yaml|yml", "", CommentYAML);

  // add external lexers
  loadExternalLexers();
}

////////////////////////////////////////////////////////////////////////////////

void StyleManager::addLexer(const QString &sid,
                            const QString &name, const QString &suff,
                            const QString &filter,
                            const CommentInfo& comment)
{
  static LexerType id = NONE;
  LexerType lt = ++id;

  QAction *act = stylesMenu->addAction(name);
  act->setData(lt);
  act->setCheckable(true);
  group->addAction(act);

  LexerInfo info;
  info.act = act;
  info.name = name;
  info.sid = sid;
  info.comm = comment;
  LexerActions[lt] = info;

  LexerNames[sid] = lt;

  QStringList suffixes = suff.split('|');
  foreach(QString str, suffixes)
  {
    LexerSuffixes[str] = lt;
  }

  if (!filter.isEmpty())
  {
    LexerFilters[filter] = lt;
  }
}

void StyleManager::setLexer(QAction* act)
{
  LexerType lt = (LexerType) act->data().toInt();
  emit activated(lt);
}

void StyleManager::setCurrentLexer(int lt)
{
  if (LexerActions.contains(lt))
    LexerActions[lt].act->setChecked(true);
  else
    LexerActions[NONE].act->setChecked(true);
}

QsciLexer* StyleManager::createLexer(int lt)
{
  QsciLexer *lex = doCreateLexer(lt);
  if (!lex)
    return 0;

  // assign API if exists
  LexerInfo &info = LexerActions[lt];
  if (apis.contains(info.sid))
  {
    //qDebug() << "set api: " << info.sid;
    lex->setAPIs(apis[info.sid]);
  }

  // assign style
  //applyCurrentStyle(lex);

  return lex;
}

QsciLexer* StyleManager::doCreateLexer(int lt)
{
  if (LexerActions.contains(lt))
  {
    const LexerInfo &info = LexerActions[lt];

    if (info.sid == "ada")
      return new QsciLexerAda;
    if (info.sid == "asm")
      return new QsciLexerASM;
    if (info.sid == "bash")
      return new QsciLexerBash;
    if (info.sid == "batch")
      return new QsciLexerBatch;
    if (info.sid == "cmake")
      return new QsciLexerCMake;
    if (info.sid == "cplusplus")
      return new QsciLexerCPP;
    if (info.sid == "csharp")
      return new QsciLexerCSharp;
    if (info.sid == "css")
      return new QsciLexerCSS;
    if (info.sid == "d")
      return new QsciLexerD;
    if (info.sid == "diff")
      return new QsciLexerDiff;
    if (info.sid == "dsl")
      return new LexerDSL;
    if (info.sid == "fortran")
      return new QsciLexerFortran;
    if (info.sid == "fortran77")
      return new QsciLexerFortran77;
    if (info.sid == "haskell")
      return new QsciLexerHaskell;
    if (info.sid == "idl")
      return new QsciLexerIDL;
    if (info.sid == "java")
      return new QsciLexerJava;
    if (info.sid == "js")
      return new QsciLexerJavaScript;
    if (info.sid == "lisp")
      return new QsciLexerLisp;
    if (info.sid == "lua")
      return new QsciLexerLua;
    if (info.sid == "makefile")
      return new QsciLexerMakefile;
    if (info.sid == "nsis")
      return new QsciLexerNSIS;
    if (info.sid == "pascal")
      return new QsciLexerPascal;
    if (info.sid == "perl")
      return new QsciLexerPerl;
    if (info.sid == "ps")
      return new QsciLexerPostScript;
    if (info.sid == "pov")
      return new QsciLexerPOV;
    if (info.sid == "props")
      return new QsciLexerProperties;
    if (info.sid == "python")
      return new QsciLexerPython;
    if (info.sid == "ruby")
      return new QsciLexerRuby;
    if (info.sid == "spice")
      return new QsciLexerSpice;
    if (info.sid == "sql")
      return new QsciLexerSQL;
    if (info.sid == "tcl")
      return new QsciLexerTCL;
    if (info.sid == "tex")
      return new QsciLexerTeX;
    if (info.sid == "verilog")
      return new QsciLexerVerilog;
    if (info.sid == "vhdl")
      return new QsciLexerVHDL;
    if (info.sid == "xdxf")
      return new QsciLexerXML;
    if (info.sid == "xml" ||
        info.sid == "asp" ||
        info.sid == "vbs" ||
        info.sid == "php" ||
        info.sid == "sgml" ||
        info.sid == "html")
      return new QsciLexerXML;
    if (info.sid == "yaml")
      return new QsciLexerYAML;

    // create external lexer
    if (ExtLexerMap.contains(info.sid))
    {
      const QString &type = ExtLexerMap[info.sid];

      if (type == "asm")
        return new ExtLexerASM(info.sid);
    }
  }

  return 0;
}

int StyleManager::lexerForFilename(const QString &filename)
{
  QString fileName(filename);

#ifdef Q_WS_WIN
  Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
  Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif

  QString suff = QFileInfo(fileName).suffix();
  if (cs == Qt::CaseInsensitive)
    suff = suff.toLower();

  if (!suff.isEmpty())
  {
    if (CustomSuffixes.contains(suff))
      return CustomSuffixes[suff];

    if (LexerSuffixes.contains(suff))
      return LexerSuffixes[suff];
  }

  if (suff.isEmpty())
  {
    fileName += '.';
  }

  foreach(QString str, CustomFilters.keys())
  {
    QRegExp re(str, cs, QRegExp::Wildcard);
    if (re.indexIn(fileName) >= 0)
    {
      return CustomFilters[str];
    }
  }

  foreach(QString str, LexerFilters.keys())
  {
    QRegExp re(str, cs, QRegExp::Wildcard);
    if (re.indexIn(fileName) >= 0)
    {
      return LexerFilters[str];
    }
  }

  return 0;
}

int StyleManager::lexerOfSID(const QString &sid)
{
  if (LexerNames.contains(sid))
  {
    return LexerNames[sid];
  }
  return 0;
}

QString StyleManager::lexerSID(int lt)
{
  if (LexerActions.contains(lt))
  {
    return LexerActions[lt].sid;
  }

  return "none";
}

LexerFiltersInfo StyleManager::lexerFilters(int lt)
{
  LexerFiltersInfo lf;

  {
    QMapIterator<QString,int> it(LexerSuffixes);
    while (it.hasNext())
    {
      it.next();
      if (it.value() == lt)
        lf.constExt.append(it.key());
    }
  }

  {
    QMapIterator<QString,int> it(LexerFilters);
    while (it.hasNext())
    {
      it.next();
      if (it.value() == lt)
        lf.constWildcard.append(it.key());
    }
  }

  {
    QMapIterator<QString,int> it(CustomSuffixes);
    while (it.hasNext())
    {
      it.next();
      if (it.value() == lt)
        lf.customExt.append(it.key());
    }
  }

  {
    QMapIterator<QString,int> it(CustomFilters);
    while (it.hasNext())
    {
      it.next();
      if (it.value() == lt)
        lf.customWildcard.append(it.key());
    }
  }

  return lf;
}

void StyleManager::setLexerCustomFilters(const QMap<int, QString>& ext,
                                         const QMap<int, QString>& wildcard)
{
  {
    CustomSuffixes.clear();
    QMapIterator<int, QString> it(ext);
    while (it.hasNext())
    {
      it.next();
      if (it.value().isEmpty())
        continue;
      CustomSuffixes[it.value()] = it.key();
    }
  }
  {
    CustomFilters.clear();
    QMapIterator<int, QString> it(wildcard);
    while (it.hasNext())
    {
      it.next();
      if (it.value().isEmpty())
        continue;
      CustomFilters[it.value()] = it.key();
    }
  }

}

void StyleManager::loadExternalLexers()
{
  QString lexPath = RESOURCE_DIR + "lexers/";

  QDir dir(lexPath);
  QStringList lexList = dir.entryList(QStringList() << "*.lex");
  foreach (QString name, lexList)
  {
    QFile mapFile(lexPath + "/" + name);
    if (!mapFile.open(QFile::ReadOnly))
      continue;

    QString key;
    QMap<QString,QString> map;

    // read content into map
    QTextStream mapTS(&mapFile);
    while (!mapTS.atEnd())
    {
      QString line(mapTS.readLine().trimmed());
      if (line.isEmpty())
        continue;

      if (line.startsWith("."))
      {
        // get control token
        QTextStream ts(&line); ts >> key; key = key.toLower();
        // get rest of the line
        line = line.mid(key.length()).trimmed();
        if (!line.isEmpty())
          map[key] = line;
        continue;
      }

      // else put the line into map
      if (!key.isEmpty())
        map[key] = QString(map[key] + " " + line).trimmed();
    }

    // check the map
    if (map.isEmpty())
      continue;

    registerExternalLexer(map);
  }
}

bool StyleManager::registerExternalLexer(QMap<QString,QString> &map)
{
  //qDebug() << map;

  CommentInfo comm;

  // check sid, type and name first
  QString sid = map[".sid"];
  QString name = map[".name"];
  QString type = map[".type"];

  if (sid.isEmpty() || name.isEmpty() || type.isEmpty())
    return false;

  // assembler lexer
  if (type == "asm")
  {
    comm = CommentASM;

    // register lexer data
    if (!ExtLexerASM::registerData(sid, map))
      return false;
  } else // unknown type
    return false;

  // register lexer
  ExtLexerMap[sid] = type;
  addLexer(sid, name, map[".ext"], map[".filter"], comm);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void StyleManager::initAPIs()
{
  // get APIs
  apiPath = RESOURCE_DIR + "apis/";

  QsciLexerCPP *tmp = new QsciLexerCPP;

  QDir apiDir(apiPath);
  QStringList apiList = apiDir.entryList(QStringList() << "*.api");
  for (int i = 0; i < apiList.count(); i++)
  {
    QString name = apiList.at(i);
    QString filename = apiPath + "/" + name;
    QsciAPIs *api = new QsciAPIs(tmp);
    if (!api->load(filename))
    {
      qDebug() << "API not loaded: " << filename;
      delete api;
      continue;
    }

    api->prepare();

    name = QFileInfo(name).baseName();
    apis[name] = api;

    //qDebug() << "API loaded: [" << name << "] from " << filename;
  }
}

////////////////////////////////////////////////////////////////////////////////

void StyleManager::initStyles()
{
  // get styles
  stylePath = RESOURCE_DIR + "styles/";

  QFile mapFile(stylePath + "styles.map");
  if (!mapFile.open(QFile::ReadOnly))
    return;

  bool ok;
  QString lang, styleid;

  QTextStream mapTS(&mapFile);
  while (!mapTS.atEnd())
  {
    QString line(mapTS.readLine().simplified());
    if (line.isEmpty())
      continue;

    if (line.startsWith("["))
    {
      line.remove("[");
      line.remove("]");
      if (line.isEmpty())
        continue;

      lang = line;
      continue;
    }

    if (line.startsWith("."))
    {
      line.remove(".");
      if (line.isEmpty())
        continue;

      styleid = line;
      continue;
    }

    QStringList sl(line.split(QRegExp("\\s+"), QString::SkipEmptyParts));
    if (sl.length() < 2)
      continue;

    int val = sl.at(1).toInt(&ok);
    if (!ok)
      continue;

    styleMap[lang][styleid].append(val);
  }
}

bool StyleManager::loadCurrentStyle(const QString &fileName)
{
  QFile mapFile(stylePath + fileName + ".stl");
  if (!mapFile.open(QFile::ReadOnly))
    return false;

  // drop styles
  currentStyle.clear();
  currentEditStyle = EditStyle();

  enum StyleType { None, Color, Paper, Bold, Italic, Underline };
  bool ok;
  QString styleid;
  StyleData sdata;

  QTextStream mapTS(&mapFile);
  while (!mapTS.atEnd())
  {
    QString line(mapTS.readLine().simplified());
    if (line.isEmpty())
      continue;

    if (line.startsWith("."))
    {
      line.remove(".");
      if (line.isEmpty())
        continue;

      styleid = line;
      sdata = StyleData();
      currentStyle[styleid] = sdata;
      continue;
    }

    QStringList sl(line.split(QRegExp("\\s+"), QString::SkipEmptyParts));

    enum StyleType styletype = None;
    QString key = sl.first().toLower();
    if (key == "color") styletype = Color; else
      if (key == "paper") styletype = Paper; else
        if (key == "bold") styletype = Bold; else
          if (key == "italic") styletype = Italic; else
            if (key == "underline") styletype = Underline; else
              continue;

    // load color
    if (styletype == Color || styletype == Paper)
    {
      if (sl.length() < 2)
        continue;

      unsigned int cval = sl.at(1).toUInt(&ok, 16);
      if (!ok)
        continue;

      QColor col(cval);
      if (!col.isValid())
        continue;

      if (styletype == Color)
        sdata.color = col;
      else
        sdata.paper = col;

      currentStyle[styleid] = sdata;

      //qDebug() << col;

      // check name
      if (styleid == "default")
      {
        if (styletype == Color)
          currentEditStyle.TextColor = col;
        else
          currentEditStyle.BackgroundColor = col;
      } else
      if (styleid == "caret_line")
      {
        if (styletype == Color)
          currentEditStyle.CaretLineForegroundColor = col;
        else
          currentEditStyle.CaretLineBackgroundColor = col;
      } else
      if (styleid == "selection")
      {
        if (styletype == Color)
          currentEditStyle.SelectionForegroundColor = col;
        else
          currentEditStyle.SelectionBackgroundColor = col;
      } else
      if (styleid == "lines_margin")
      {
        if (styletype == Color)
          currentEditStyle.MarginsForegroundColor = col;
        else
          currentEditStyle.MarginsBackgroundColor = col;
      } else
      if (styleid == "fold_margin")
      {
        if (styletype == Color)
          currentEditStyle.FoldMarginForegroundColor = col;
        else
          currentEditStyle.FoldMarginBackgroundColor = col;
      } else
        if (styleid == "fold_markers")
        {
          if (styletype == Color)
            currentEditStyle.FoldMarkersForegroundColor = col;
          else
            currentEditStyle.FoldMarkersBackgroundColor = col;
        } else
      if (styleid == "matched_brace")
      {
        if (styletype == Color)
          currentEditStyle.MatchedBraceForegroundColor = col;
        else
          currentEditStyle.MatchedBraceBackgroundColor = col;
      } else
      if (styleid == "unmatched_brace")
      {
        if (styletype == Color)
          currentEditStyle.UnmatchedBraceForegroundColor = col;
        else
          currentEditStyle.UnmatchedBraceBackgroundColor = col;
      } else
      if (styleid == "indent_guides")
      {
        if (styletype == Color)
          currentEditStyle.IndentationGuidesForegroundColor = col;
        else
          currentEditStyle.IndentationGuidesBackgroundColor = col;
      } else
      if (styleid == "edge")
      {
        if (styletype == Color)
          currentEditStyle.EdgeColor = col;
      }

      continue;
    }

    // load bool
    switch (styletype)
    {
    case Bold:  sdata.bold = true; break;
    case Italic:  sdata.italic = true; break;
    case Underline:  sdata.underline = true; break;
    default:;
    }
    currentStyle[styleid] = sdata;
  }

  return true;
}

void StyleManager::applyCurrentStyle(QsciLexer *lexer, const QFont &font)
{
  if (!lexer)
    return;

  lexer->setColor(currentEditStyle.TextColor, -1);
  lexer->setPaper(currentEditStyle.BackgroundColor, -1);
  lexer->setFont(font, -1);
  lexer->setDefaultColor(currentEditStyle.TextColor);
  lexer->setDefaultPaper(currentEditStyle.BackgroundColor);
  lexer->setDefaultFont(font);

  QString id = lexer->lexer();
  if (id.isEmpty())
    id = lexer->language();

  //qDebug() << "applyCurrentStyle   " << id;

  if (!styleMap.contains(id))
    return;

  //qDebug() << "applyCurrentStyle proceeding";

  const StyleMapItems &smi = styleMap[id];
  StyleMapItems::const_iterator it = smi.constBegin();
  for (; it != smi.constEnd(); it++)
  {
    //qDebug() << "item: " << it.key();

    if (!currentStyle.contains(it.key()))
        continue;

    const StyleData &sd = currentStyle[it.key()];
    const QList<int> &list = it.value();

    QFont f(font);
    f.setBold(sd.bold);
    f.setItalic(sd.italic);
    f.setUnderline(sd.underline);

    for (int i = 0; i < list.count(); i++)
    {
      int idx = list.at(i);

      //qDebug() << sd.color << " " << list.at(i);
      lexer->setFont(f, idx);

      if (sd.color.isValid())
        lexer->setColor(sd.color, idx);

      if (sd.paper.isValid())
        lexer->setPaper(sd.paper, idx);
    }
  }

  lexer->refreshProperties();
}

QStringList StyleManager::styles()
{
  QDir dir(stylePath);
  QStringList list = dir.entryList(QStringList() << "*.stl");
  return list;
}

////////////////////////////////////////////////////////////////////////////////

void StyleManager::readConfig(QSettings &set)
{
  QString colorScheme = set.value("colorScheme", "STE").toString();
  loadCurrentStyle(colorScheme);

  set.beginGroup("CustomSuffixes");
  CustomSuffixes.clear();
  QStringList keys = set.allKeys();
  foreach (QString key, keys)
  {
    QString sid = set.value(key).toString();
    int lt = lexerOfSID(sid);
    if (!lt) continue;
    CustomSuffixes[key] = lt;
  }
  set.endGroup();

  set.beginGroup("CustomFilters");
  CustomFilters.clear();
  keys = set.allKeys();
  foreach (QString key, keys)
  {
    QString sid = set.value(key).toString();
    int lt = lexerOfSID(sid);
    if (!lt) continue;
    CustomFilters[key] = lt;
  }
  set.endGroup();
}

void StyleManager::writeConfig(QSettings &set)
{
  set.beginGroup("CustomSuffixes");
  set.remove("");
  {
    QMapIterator<QString,int> it(CustomSuffixes);
    while (it.hasNext())
    {
      it.next();
      set.setValue(it.key(), lexerSID(it.value()));
    }
  }
  set.endGroup();

  set.beginGroup("CustomFilters");
  set.remove("");
  {
    QMapIterator<QString,int> it(CustomFilters);
    while (it.hasNext())
    {
      it.next();
      set.setValue(it.key(), lexerSID(it.value()));
    }
  }
  set.endGroup();
}
