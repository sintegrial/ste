#ifndef ABSTRACTDICT_H
#define ABSTRACTDICT_H

#include <QtGui>

struct DictViewInfo;
class DictView;
class QsciScintilla;

class AbstractDict : public QObject
{
  Q_OBJECT
public:
    AbstractDict(QObject *parent = 0) ;
    virtual ~AbstractDict();

    virtual QWidget* output(QsciScintilla *editor, const QString &baseDir);

    static QString& removeHtml(QString &str);

    static const QString styleSheet;

private slots:
    void prev();
    void next();
    void home();
    void end();

protected:
    virtual void createIndex() = 0;
    virtual void render() = 0;

    static const QString exampleStart, exampleEnd;
    static const QString idiomStart, idiomEnd;
    static const QString commentStart, commentEnd;
    static const QString highlightStart, highlightEnd;
    static const QString strongStart, strongEnd;
    static const QString phraseStart, phraseEnd;
    static const QString translationStart, translationEnd;
    static const QString transcriptionStart, transcriptionEnd;
    static const QString emphasisStart, emphasisEnd;

    const QString refStart(const QString &ref);
    const QString urlStart(const QString &ref);

    const QString multimediaStart(const QString &mm);
    static QList<QByteArray> imagesExt, soundsExt;

    const QString abbrStart(const QString &abbr);

protected:
    QMap<QString, QString> m_abbr;

    QsciScintilla *myEditor;
    DictViewInfo *myInfo;
    DictView *myView;

    int myItemsOnPage;
};

#endif // ABSTRACTDICT_H
