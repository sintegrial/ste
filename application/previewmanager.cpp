#include "previewmanager.h"
#include "previewmanagerdialog.h"

#include "previewers/htmlrender.h"
#include "previewers/dslrender.h"
#include "previewers/xdxfrender.h"

PreviewManager::PreviewManager(QObject *parent) :
    QObject(parent)
{
  previewDialog = new PreviewManagerDialog(dynamic_cast<QWidget*>(parent));
}

bool PreviewManager::preview(QsciScintilla *editor, const QString &sid, const QString &filename)
{
  QWidget *output = 0;

  QTime tick; tick.start();

  QFileInfo fi(filename);
  QString ext(fi.suffix().toLower());
  QString dir("file:///" + fi.absolutePath() + "/");

  // only accept known items

  if (sid == "html" || sid == "php" || ext.contains("htm") || ext.contains("php"))
  {
    HTMLRender r;
    output = r.output(editor, dir);
    qDebug() << "render: " << tick.elapsed();
    previewDialog->preview(output);
    return true;
  }

  if (sid == "dsl" || ext == "dsl")
  {
    DSLRender r;
    output = r.output(editor, dir);
    qDebug() << "render: " << tick.elapsed();
    previewDialog->preview(output);
    return true;
  }

  if (ext == "xdxf")
  {
    XDXFRender r;
    output = r.output(editor, dir);
    qDebug() << "render: " << tick.elapsed();
    previewDialog->preview(output);
    return true;
  }

  return false;
}

// config
void PreviewManager::readConfig(QSettings &set)
{
  previewDialog->restoreGeometry(
      set.value("PreviewDialog", previewDialog->frameGeometry()).toByteArray()
      );
}

void PreviewManager::writeConfig(QSettings &set)
{
  set.setValue("PreviewDialog", previewDialog->saveGeometry());
}
