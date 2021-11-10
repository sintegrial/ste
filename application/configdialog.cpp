#include "configdialog.h"
#include "ui_configdialog.h"

#include "config.h"
#include "stylemanager.h"
#include "translationmanager.h"

ConfigDialog::ConfigDialog(const ConfigInit &init, QWidget *parent) :
    QDialog(parent, Qt::WindowStaysOnTopHint),
    ui(new Ui::ConfigDialog),
    config(0)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);
    ui->labelHead->hide();

    ui->cbDefCodecs->addItems(*init.codecs);

    for (int i = 0; i < init.eols.count(); i++)
    {
      QAction *act = init.eols.at(i);
      ui->cbDefEOLs->addItem(act->icon(), act->text(), act->data());
    }
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

int ConfigDialog::exec(Config *config)
{
  static bool init = false;

  this->config = config;

  if (!init)
  {
    init = true;

    // color schemes
    QStringList schemes = StyleManager::instance()->styles();
    for (int i = 0; i < schemes.count(); i++)
      schemes[i] = schemes[i].remove(".stl");
    ui->cbColorScheme->addItems(schemes);

    // lexers
    QList<LexerInfo> lexers = StyleManager::instance()->lexers();
    for (int i = 0; i < lexers.count(); i++)
    {
      const LexerInfo &li = lexers.at(i);
      if (li.name.isEmpty())
        continue;
      int lt = li.act->data().toInt();
      ui->cbLexer->addItem(li.name, lt);
      // load custom filters
      LexerFiltersInfo lf = StyleManager::instance()->lexerFilters(lt);
      config->customExt[lt] = lf.customExt.join(" ");
      config->customWildcard[lt] = lf.customWildcard.join(" ");
    }

    ui->cbLexer->setCurrentIndex(0);

    // languages
    const QMap<QString, QString>& languages = TranslationManager::instance()->languages();
    QMapIterator<QString, QString> it(languages);
    while (it.hasNext())
    {
      it.next();
      ui->cbLang->addItem(TranslationManager::instance()->icon(it.key()), it.value(), it.key());
    }
  }

  // reload config
  discard();

  //return QDialog::exec();
  show();
  return 1;
}

void ConfigDialog::discard()
{
  ui->sbTabSize->setValue(config->tabSize);
  ui->sbIndentSize->setValue(config->indentSize);
  ui->cbSpaceTabs->setChecked(config->spaceTabs);
  ui->cbIndentTabs->setChecked(config->tabIndents);
  ui->cbAutoIndent->setChecked(config->autoIndent);
  ui->cbBackspaceUnindent->setChecked(config->backspaceUnindent);

  ui->cbEdge->setChecked(config->lineEdge);
  ui->sbEdgePos->setValue(config->lineEdgeSize);

  ui->sbFontSize->setValue(config->fontSize);
  ui->cbFont->setCurrentFont(QFont(config->font));

  ui->cbFolding->setChecked(config->foldStyle);
  if (config->foldStyle)
    ui->cbFoldStyle->setCurrentIndex(config->foldStyle-1);

  ui->cbCompAPI->setChecked(config->autoCompletion == QsciScintilla::AcsAll
                            || config->autoCompletion == QsciScintilla::AcsAPIs);
  ui->cbCompSource->setChecked(config->autoCompletion == QsciScintilla::AcsAll
                            || config->autoCompletion == QsciScintilla::AcsDocument);
  ui->cbCompCS->setChecked(config->autoCompletionCaseSensitive);
  ui->cbCompRW->setChecked(config->autoCompletionReplaceWord);

  ui->cbInsertPairs->setChecked(config->insertPairs);

  int i = ui->cbDefCodecs->findText(config->defaultCodec);
  ui->cbDefCodecs->setCurrentIndex(i);
  i = ui->cbDefEOLs->findData(config->defaultEOL);
  ui->cbDefEOLs->setCurrentIndex(i);
  ui->cbDefBOM->setChecked(config->defaultBOM);

  ui->cbTrimOnSave->setChecked(config->trimOnSave);
  ui->cbAddLineOnSave->setChecked(config->addLineOnSave);

  // color schemes
  ui->cbColorScheme->setCurrentIndex(ui->cbColorScheme->findText(config->colorScheme));

  // lexers
  int lt = ui->cbLexer->itemData(ui->cbLexer->currentIndex()).toInt();

  LexerFiltersInfo lf = StyleManager::instance()->lexerFilters(lt);
  config->customExt[lt] = lf.customExt.join(" ");
  config->customWildcard[lt] = lf.customWildcard.join(" ");

  on_cbLexer_currentIndexChanged(ui->cbLexer->currentIndex());

  // lang
  i = ui->cbLang->findData(config->language);
  ui->cbLang->setCurrentIndex(i);
}

void ConfigDialog::on_cbLexer_currentIndexChanged(int idx)
{
  int lt = ui->cbLexer->itemData(idx).toInt();
  LexerFiltersInfo lf = StyleManager::instance()->lexerFilters(lt);
  ui->tePredefinedExt->setPlainText(lf.constExt.join(" "));
  ui->tePredefinedWildcard->setPlainText(lf.constWildcard.join(" "));
  ui->teCustomExt->setPlainText(config->customExt[lt]);
  ui->teCustomWildcard->setPlainText(config->customWildcard[lt]);
}

void ConfigDialog::on_teCustomExt_textChanged()
{
  int lt = ui->cbLexer->itemData(ui->cbLexer->currentIndex()).toInt();
  config->customExt[lt] = ui->teCustomExt->toPlainText();
}

void ConfigDialog::on_teCustomWildcard_textChanged()
{
  int lt = ui->cbLexer->itemData(ui->cbLexer->currentIndex()).toInt();
  config->customWildcard[lt] = ui->teCustomWildcard->toPlainText();
}


void ConfigDialog::apply()
{
  config->tabSize = ui->sbTabSize->value();
  config->indentSize = ui->sbIndentSize->value();
  config->spaceTabs = ui->cbSpaceTabs->isChecked();
  config->tabIndents = ui->cbIndentTabs->isChecked();
  config->autoIndent = ui->cbAutoIndent->isChecked();
  config->backspaceUnindent = ui->cbBackspaceUnindent->isChecked();

  config->lineEdge = ui->cbEdge->isChecked();
  config->lineEdgeSize = ui->sbEdgePos->value();

  config->fontSize = ui->sbFontSize->value();
  config->font = ui->cbFont->currentFont().family();

  config->foldStyle = ui->cbFolding->isChecked() ?
                      ui->cbFoldStyle->currentIndex()+1 : 0;

  if (ui->cbCompAPI->isChecked())
  {
    if (ui->cbCompSource->isChecked())
      config->autoCompletion = QsciScintilla::AcsAll;
    else
      config->autoCompletion = QsciScintilla::AcsAPIs;
  } else
    if (ui->cbCompSource->isChecked())
      config->autoCompletion = QsciScintilla::AcsDocument;
    else
      config->autoCompletion = QsciScintilla::AcsNone;

  config->autoCompletionCaseSensitive = ui->cbCompCS->isChecked();
  config->autoCompletionReplaceWord = ui->cbCompRW->isChecked();

  config->insertPairs = ui->cbInsertPairs->isChecked();

  config->defaultEOL = ui->cbDefEOLs->itemData(ui->cbDefEOLs->currentIndex()).toInt();
  config->defaultCodec = ui->cbDefCodecs->currentText();
  config->defaultBOM = ui->cbDefBOM->isChecked();

  config->trimOnSave = ui->cbTrimOnSave->isChecked();
  config->addLineOnSave = ui->cbAddLineOnSave->isChecked();

  config->colorScheme = ui->cbColorScheme->currentText();

  config->language = ui->cbLang->itemData(ui->cbLang->currentIndex()).toString();

  emit configChanged();
}

void ConfigDialog::accept()
{
  apply();
  QDialog::accept();
}

void ConfigDialog::on_buttonBox_clicked ( QAbstractButton * button )
{
  switch (ui->buttonBox->standardButton(button))
  {
  case QDialogButtonBox::Discard:
    discard();
    break;

  case QDialogButtonBox::Apply:
    apply();
    break;

  default:;
  }
}
