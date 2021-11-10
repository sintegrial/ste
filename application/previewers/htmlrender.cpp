#include "htmlrender.h"
#include "utils.h"

#include "Qsci/qsciscintilla.h"

#include <QtWebKit>

HTMLRender::HTMLRender(QObject *parent) :
    QObject(parent)
{
}

QWidget* HTMLRender::output(QsciScintilla *editor, const QString &baseDir)
{
  QWebView *view = new QWebView();
  view->setHtml(editor->text(), QUrl(baseDir));
  return view;
}
