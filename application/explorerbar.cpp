#include "explorerbar.h"
#include "ui_explorerbar.h"

#include "utils.h"
#include "utilsui.h"

ExplorerBar::ExplorerBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExplorerBar)
{
    ui->setupUi(this);
    setObjectName("ExplorerBar");

    layout()->setMargin(1);

    model = new QFileSystemModel(this);
    model->setRootPath("");
    model->setNameFilterDisables(false);

    //model->setFilter(QDir::AllDirs | QDir::Files | QDir::Drives);

    ui->tvDirs->setModel(model);

    connect(ui->tvDirs->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentRowChanged(QModelIndex,QModelIndex)));

    ui->tvDirs->setMouseTracking(true);
    connect(ui->tvDirs, SIGNAL(entered(QModelIndex)), this, SLOT(highlighted(QModelIndex)));

    connect(ui->tbSynchReq, SIGNAL(clicked()), this, SIGNAL(synchFileRequest()));

    connect(ui->leDir->lineEdit(), SIGNAL(returnPressed()), this, SLOT(leDir_returnPressed()));

    ui->tvDirs->installEventFilter(this);


    QCompleter *comp = new QCompleter(this);
    comp->setCompletionMode(QCompleter::InlineCompletion);
    ui->leFilter->setCompleter(comp);

    QCompleter *comp2 = new QCompleter(this);
    comp2->setCompletionMode(QCompleter::InlineCompletion);
    ui->leDir->setCompleter(comp2);


    ui->tvDirs->addAction(ui->actionOpen);
    QAction *sep = new QAction(this); sep->setSeparator(true);
    ui->tvDirs->addAction(sep);
    ui->tvDirs->addAction(ui->actionCopyPath);
    ui->tvDirs->addAction(ui->actionCopyName);
}

ExplorerBar::~ExplorerBar()
{
    delete ui;
}

void ExplorerBar::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ExplorerBar::readConfig(QSettings &set)
{
  set.beginGroup("ExplorerBar");

  navigate(set.value("path").toString());

  ui->tbSynch->setChecked(set.value("synch", true).toBool());

  UtilsUI::loadCombo(set, "dirCompleter", ui->leDir);
  UtilsUI::loadCombo(set, "filterCompleter", ui->leFilter);

  ui->leFilter->setEditText(set.value("filter").toString());

  set.endGroup();
}

void ExplorerBar::writeConfig(QSettings &set)
{
  set.beginGroup("ExplorerBar");

  set.setValue("path", model->filePath(ui->tvDirs->currentIndex()));
  set.setValue("synch", ui->tbSynch->isChecked());

  set.setValue("filter", ui->leFilter->currentText());

  UtilsUI::storeCombo(set, "dirCompleter", ui->leDir);
  UtilsUI::storeCombo(set, "filterCompleter", ui->leFilter);

  set.endGroup();
}

void ExplorerBar::doSetPath(const QString &name, bool select)
{
  QString path = Utils::normalizeFilename(name);
  QModelIndex index = model->setRootPath(path);
  ui->tvDirs->setRootIndex(index);
  ui->leDir->lineEdit()->setText(path);

  // select the 1st item by default
  if (select)
  QTimer::singleShot(100, this, SLOT(doSelectFirst()));
}

void ExplorerBar::doSelectFirst()
{
  if (model->canFetchMore(ui->tvDirs->rootIndex()))
    QTimer::singleShot(100, this, SLOT(doSelectFirst()));

  ui->tvDirs->setCurrentIndex(ui->tvDirs->rootIndex().child(0,0));
}

void ExplorerBar::navigate(const QString &name)
{
  QFileInfo fi(name);
  if (name.isEmpty() || (fi.isDir() && fi.exists()))
  {
    doSetPath(name);
    return;
  }

  QString path = fi.absolutePath();
  QFileInfo di(path);
  if (di.exists())
  {
    doSetPath(path, false);
    ui->tvDirs->setCurrentIndex(model->index(name));
  }
}

void ExplorerBar::on_tbSynch_clicked()
{
  if (ui->tbSynch->isChecked())
    emit synchFileRequest();
}

void ExplorerBar::synchronizeIfNeeded(const QString &name)
{
  if (ui->tbSynch->isChecked())
    navigate(name);
}

void ExplorerBar::currentRowChanged(const QModelIndex & current, const QModelIndex & )
{
  emit fileCurrent(Utils::normalizeFilename(model->filePath(current)));
}

void ExplorerBar::highlighted(const QModelIndex & current)
{
  emit fileCurrent(Utils::normalizeFilename(model->filePath(current)));
}

void ExplorerBar::on_tbPickItem_clicked()
{
  QString name = QFileDialog::getExistingDirectory(
      this, tr("Path to the directory to browse for:"),
      model->filePath(ui->tvDirs->currentIndex()));

  if (!name.isEmpty())
  {
    doSetPath(name);
    ui->leDir->lineEdit()->selectAll();
  }
}

void ExplorerBar::on_tvDirs_activated(const QModelIndex & index)
{
  QString path = model->filePath(index);

  if (model->isDir(index))
  {
    doSetPath(path);
    return;
  }

  emit fileClicked(path);
}

void ExplorerBar::leDir_returnPressed()
{
  QString name = ui->leDir->currentText();
  QFileInfo fi(name);
  if (!fi.exists() || !fi.isDir())
  {
    QModelIndex index = ui->tvDirs->rootIndex();
    name = model->filePath(index);
    model->setRootPath(name);
    ui->leDir->lineEdit()->setText(name);
    return;
  }

  doSetPath(name);
}

void ExplorerBar::on_tbUp_clicked()
{
  QModelIndex old = ui->tvDirs->rootIndex();

  QModelIndex index = model->parent(ui->tvDirs->rootIndex());
  //qDebug() << index;
  if (index.isValid())
  {
    doSetPath(model->filePath(index), false);
    ui->tvDirs->setCurrentIndex(old);
  }
  else
  {
    doSetPath("");
  }
}

void ExplorerBar::on_tbRoot_clicked()
{
  QString path = model->filePath(ui->tvDirs->rootIndex());

#ifdef Q_WS_WIN
  if (path.length() > 3)
    navigate(Utils::driveName(path));
  else
    navigate("");
  return;
#endif

#ifdef Q_WS_X11
  navigate("/");
  return;
#endif
}

bool ExplorerBar::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::KeyPress)
  {
    if (obj == ui->tvDirs)
    {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      switch (keyEvent->key())
      {
      case Qt::Key_Backspace:
        on_tbUp_clicked();
        return true;
      case Qt::Key_Home:
        if (keyEvent->modifiers() == Qt::ControlModifier)
        {
          on_tbRoot_clicked();
          return true;
        }
      }
    }
  }

  return QObject::eventFilter(obj, event);
}

void ExplorerBar::on_leFilter_textChanged ( const QString & text )
{
  model->setNameFilters(text.split(";", QString::SkipEmptyParts));
}

void ExplorerBar::on_actionOpen_triggered()
{
  on_tvDirs_activated(ui->tvDirs->currentIndex());
}

void ExplorerBar::on_actionCopyPath_triggered()
{
  QString name(model->filePath(ui->tvDirs->currentIndex()));
  if (!name.isEmpty())
  {
    QApplication::clipboard()->setText(
        Utils::normalizeFilename(name)
        );
  }
}

void ExplorerBar::on_actionCopyName_triggered()
{
  QString name(model->filePath(ui->tvDirs->currentIndex()));
  if (!name.isEmpty())
  {
    QApplication::clipboard()->setText(
        Utils::normalizeFilename(QFileInfo(name).fileName())
        );
  }
}
