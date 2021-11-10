#ifndef STYLES_H
#define STYLES_H

#include <QtGui>
#include <Qsci/qscilexer.h>
#include <Qsci/qsciscintilla.h>
#include <Qsci/qsciapis.h>

#include "editstyle.h"

typedef int LexerType;
const LexerType NONE = 0;

////////////////////////////////////////////////////////////////////////////////

struct StyleData
{
  QColor color, paper;
  bool bold, italic, underline;

  StyleData() :
      color(Qt::transparent),
  //paper(QColor()),
      bold(false), italic(false), underline(false)
  {
  }
};

typedef QMap<QString, QList<int> > StyleMapItems;

////////////////////////////////////////////////////////////////////////////////

struct CommentInfo
{
  CommentInfo(const QString &markSC_ = "",
              const QString &markMC1_ = "",
              const QString &markMC2_ = "") :
      markSC(markSC_), markMC1(markMC1_), markMC2(markMC2_)
  {
  }

  QString markSC, markMC1, markMC2;
};

////////////////////////////////////////////////////////////////////////////////

struct LexerInfo
{
  QAction *act;
  QString name;
  QString sid;
  CommentInfo comm;
};

struct LexerFiltersInfo
{
  QStringList constExt, constWildcard;
  QStringList customExt, customWildcard;
};

////////////////////////////////////////////////////////////////////////////////

class StyleManager : public QObject
{
  Q_OBJECT

  static StyleManager * This;

public:
  StyleManager(QWidget *parent);
  ~StyleManager();

  static StyleManager* instance() { return This; }

  QMenu* menu() { return stylesMenu; }
  void setMenu(QMenu *menu);

  // lexers
  void setCurrentLexer(int lt);

  int lexerForFilename(const QString &fileName);
  int lexerOfSID(const QString &sid);
  QString lexerSID(int lt);

  QsciLexer* createLexer(int lt);

  QList<LexerInfo> lexers() const { return LexerActions.values(); }
  const LexerInfo lexerInfo(int lt) const { return LexerActions[lt]; }
  LexerFiltersInfo lexerFilters(int lt);
  void setLexerCustomFilters(const QMap<int, QString>& ext, const QMap<int, QString>& wildcard);

  // styles
  bool loadCurrentStyle(const QString &fileName);
  void applyCurrentStyle(QsciLexer *lexer, const QFont &font);

  inline const EditStyle& editStyle() const { return currentEditStyle; }

  QStringList styles();

  // config
  void readConfig(QSettings &set);
  void writeConfig(QSettings &set);

signals:
  void activated(int lt);

private slots:
  void setLexer(QAction* act);

private:
  void addLexer(const QString &sid, const QString &name, const QString &suff,
                const QString &filter = "", const CommentInfo& comment = CommentInfo());
  QsciLexer* doCreateLexer(int lt);

  void loadExternalLexers();
  bool registerExternalLexer(QMap<QString,QString> &map);

  void initAPIs();
  void initStyles();

  QMenu *stylesMenu;

  QActionGroup *group;
  QMap<int,LexerInfo> LexerActions;
  QMap<QString,int> LexerSuffixes, CustomSuffixes;
  QMap<QString,int> LexerFilters, CustomFilters;
  QMap<QString,int> LexerNames;
  QMap<QString,QString> ExtLexerMap;

  QString apiPath;
  QMap<QString,QsciAPIs*> apis;

  QString stylePath;
  QMap<QString, StyleData> currentStyle;
  QMap<QString, StyleMapItems> styleMap;

  EditStyle currentEditStyle;
};

#endif // STYLES_H
