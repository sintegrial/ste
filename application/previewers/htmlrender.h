#ifndef HTMLRENDER_H
#define HTMLRENDER_H

#include <QtGui>

class QsciScintilla;

class HTMLRender : public QObject
{
Q_OBJECT
public:
    explicit HTMLRender(QObject *parent = 0);

    QWidget* output(QsciScintilla *editor, const QString &baseDir);

signals:

public slots:

};

#endif // HTMLRENDER_H
