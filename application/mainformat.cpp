#include "mainwindow.h"
#include "editwindow.h"
#include "formatmanager.h"

void MainWindow::formatCode(QAction* act)
{
  int type = formatManager->typeFromAction(act);
  if (type < 0) {
    statusBar()->showMessage(tr("Text formatting not performed"), 2000);
    return;
  }

  QString text = textEdit->hasSelectedText() ? textEdit->selectedText() : textEdit->text();

  bool res = formatManager->format((FormatManager::FormatterType)type, text, act->data());
  if (res)
  {
    QsciScintilla::EolMode eoln = textEdit->eolMode();
    textEdit->beginUndoAction();

    if (!textEdit->hasSelectedText())
      textEdit->selectAll();
    textEdit->replaceSelection(text);

    // preserve EOLs
    textEdit->convertEols(eoln);

    textEdit->endUndoAction();

    statusBar()->showMessage(tr("Text formatting completed"), 2000);
  }
  else
  {
    statusBar()->showMessage(tr("Text formatting not performed"), 2000);
  }
}
