#include "symbolsbar.h"
#include "ui_symbolsbar.h"

////////////////////////////////////////////////////////////////////////////////

const char* htmls[256] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 0
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 16
  0,0,"&quot;",0,0,0,"&amp;",0,0,0,0,0,0,0,0,0,  // 32
  0,0,0,0,0,0,0,0,0,0,0,0,"&lt;",0,"&gt;",0,  // 48
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 64
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 80
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 96
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 112
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 128
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 144
  "&nbsp;","&iexcl;","&cent;","&pound;","&curren;","&yen;","&brvbar;","&sect;",
  "&uml;","&copy;","&ordf;","&laquo;","&not;","&shy;","&reg;","&macr;",  // 160
  "&deg;","&plusmn;","&sup2;","&sup3;","&acute;","&micro;","&para;","&middot;",
  "&cedil;","&sup1;","&ordm;","&raquo;","&frac14;","&frac12;","&frac34;","&iquest;",  // 176
  "&Agrave;","&Aacute;","&Acirc;","&Atilde;","&Auml;","&Aring;","&AElig;","&Ccedil;",
  "&Egrave;","&Eacute;","&Ecirc;","&Euml;","&Igrave;","&Iacute;","&Icirc;","&Iuml;",  // 192
  "&ETH;","&Ntilde;","&Ograve;","&Oacute;","&Ocirc;","&Otilde;","&Ouml;","&times;",
  "&Oslash;","&Ugrave;","&Uacute;","&Ucirc;","&Uuml;","&Yacute;","&THORN;","&szlig;",  // 208
  "&agrave;","&aacute;","&acirc;","&atilde;","&auml;","&aring;","&aelig;","&ccedil;",
  "&egrave;","&eacute;","&ecirc;","&euml;","&igrave;","&iacute;","&icirc;","&iuml;",
  "&eth;","&ntilde;","&ograve;","&oacute;","&ocirc;","&otilde;","&ouml;","&divide;",
  "&oslash;","&ugrave;","&uacute;","&ucirc;","&uuml;","&yacute;","&thorn;","&yuml;"
};

SymbolsModel::SymbolsModel(QObject *parent) :
    QAbstractTableModel(parent),
    _unicode(true)
{

}

void SymbolsModel::setUnicode(int u)
{
  _unicode = u;
  reset();
}

int SymbolsModel::rowCount ( const QModelIndex & parent ) const
{
  return _unicode ? 65536 : 256;
}

int SymbolsModel::columnCount ( const QModelIndex & parent ) const
{
  return 4;
}

QVariant SymbolsModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section)
    {
      case 0: return tr("Code");
      case 1: return tr("Char");
      case 2: return tr("Hex");
      case 3: return tr("Html");
    }
  }

  return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant SymbolsModel::data ( const QModelIndex & index, int role ) const
{
  int id = index.row();

  if (role == Qt::DisplayRole)
  {
    switch (index.column())
    {
    case 0:
      return QString::number(id);
    case 1:
      return QChar(id);
    case 2:
      return QString("0x%1").arg(QString::number(id,16),4,'0');
    case 3:
      if (id <= 255) {
        //qDebug() << id << " " << *htmls[id];
        return htmls[id] ? QString(htmls[id]) : "";
      }
    }
  }

  if (role == Qt::BackgroundColorRole)
  {
    if (index.column() == 0)
      return Qt::lightGray;
  }

  if (role == Qt::TextColorRole)
  {
    QChar cid(id);
    if (cid.isDigit())
      return Qt::darkBlue;
    if (cid.isLetter())
      return Qt::darkGreen;
    if (cid.isMark())
      return Qt::darkRed;
    if (cid.isSymbol())
      return Qt::black;
    return Qt::darkGray;
  }

  return QVariant();
}

////////////////////////////////////////////////////////////////////////////////

SymbolsBar::SymbolsBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SymbolsBar)
{
    ui->setupUi(this);
    layout()->setMargin(1);

    myModel = new SymbolsModel(this);
    ui->tvSymbols->setModel(myModel);
    ui->tvSymbols->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    //ui->tvSymbols->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}

SymbolsBar::~SymbolsBar()
{
    delete ui;
}

void SymbolsBar::changeEvent(QEvent *e)
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

void SymbolsBar::on_sbSymbol_editingFinished()
{
  int id = ui->sbSymbol->value();
  ui->tvSymbols->selectRow(id);
  ui->tvSymbols->scrollTo(myModel->index(id, QAbstractItemView::PositionAtCenter));
}

void SymbolsBar::on_tvSymbols_activated ( const QModelIndex & index )
{
  //ui->sbSymbol->setValue(index.row());
  QString txt = myModel->data(index).toString();
  if (txt.isEmpty())
    return;

  emit insertSymbol(txt);
}

void SymbolsBar::on_tbUnicode_toggled(bool on)
{
  ui->tvSymbols->setUpdatesEnabled(false);

  myModel->setUnicode(on);

  ui->tvSymbols->setUpdatesEnabled(true);
}


void SymbolsBar::readConfig(QSettings &set)
{
  set.beginGroup("SymbolsBar");

  ui->tbUnicode->setChecked(set.value("unicode", true).toBool());
  ui->tvSymbols->scrollTo(
      myModel->index(set.value("top").toInt(), 0), QAbstractItemView::PositionAtTop
      );

  set.endGroup();
}

void SymbolsBar::writeConfig(QSettings &set)
{
  set.beginGroup("SymbolsBar");

  set.setValue("unicode", ui->tbUnicode->isChecked());
  set.setValue("top", ui->tvSymbols->indexAt(QPoint(0,0)).row());

  set.endGroup();
}

