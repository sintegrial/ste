#ifndef FORMATTING_H
#define FORMATTING_H

#include <QtGui>

struct FormatterInfo
{
  FormatterInfo() : enabled(false), menu(0)
  {
  }

  bool enabled;
  QString path;
  QMenu *menu;
};

class FormatManager : public QObject
{
  Q_OBJECT

  static const int FT_COUNT = 4;

public:
  enum FormatterType {
    FT_ASTYLE,
    FT_UNCRUSTIFY,
    FT_CSSTIDY,
    FT_HTMLTIDY
  };

    FormatManager(QObject *parent);
    ~FormatManager();

    bool available(FormatterType type) const { return formatInfo[type].enabled; }
    QMenu* menu(FormatterType type) const { return formatInfo[type].menu; }

    int typeFromAction(QAction *act) const
    { return actMap.contains(act) ? actMap[act] : -1; }

    bool format(FormatterType type, QString &text, const QVariant &data);

    static bool writeFile(const QString &fileName, const QString &data);
    static bool readFile(const QString &fileName, QString &data);

private:
    void registerActions();

    int readFormatConfigs(FormatterInfo &info,
                                         const QString &dir,
                                         const QStringList &filter);

    void initAstyle();
    void initUncrustify();
    void initCSSTidy();
    void initHTMLTidy();

    bool formatAstyle(QString &in, QString lang);
    bool formatUncrustify(QString &in, QString lang);
    bool formatCSSTidy(QString &in, QString lang);
    bool formatHTMLTidy(QString &in, QString lang);

    QString formatPath;

    //QStringList uncrFormats;

    FormatterInfo formatInfo[FT_COUNT];
    QMap<QAction*, FormatterType> actMap;
};

#endif // FORMATTING_H
