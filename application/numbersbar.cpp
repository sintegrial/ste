#include "numbersbar.h"
#include "ui_numbersbar.h"

#define BAD_NUMBER_SHEET "background-color: #ffe0e0;"


class BinModel : public QAbstractTableModel
{
public:
  BinModel(QObject *parent) : QAbstractTableModel(parent)
  {
    myBits = 0;
  }

  int rowCount ( const QModelIndex & parent = QModelIndex() ) const
  { return 1; }

  int columnCount ( const QModelIndex & parent = QModelIndex() ) const
  { return myBits; }

  virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const
  {
    if (role == Qt::CheckStateRole)
      return (myValue >> (myBits-index.column()-1)) & 1;

    return "";
  }

  Qt::ItemFlags flags ( const QModelIndex & index ) const
  {
    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const
  {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return QString::number(myBits-section-1);

    return QAbstractTableModel::headerData(section, orientation, role);
  }

  void reset()
  {
    QAbstractTableModel::reset();
  }

  int myBits;
  qulonglong myValue;
};


NumbersBar::NumbersBar(QWidget *parent) :
    SideBarBase(parent),
    ui(new Ui::NumbersBar),
    mInput(0),
    mInputValid(true)
{
    ui->setupUi(this);
    setObjectName("NumbersBar");
    setupBar(tr("Format Number"), true);

    ui->cbTypeIn->clear();
    ui->cbTypeIn->addItem("Auto", 0);
    ui->cbTypeIn->addItem("Dec", 10);
    ui->cbTypeIn->addItem("Hex", 16);
    ui->cbTypeIn->addItem("Bin", 2);

    ui->cbSizeIn->clear();
    ui->cbSizeIn->addItem("8", 8);
    ui->cbSizeIn->addItem("16", 16);
    ui->cbSizeIn->addItem("32", 32);
    ui->cbSizeIn->addItem("64", 64);
    ui->cbSizeIn->setCurrentIndex(3);

    connect(ui->cbSizeIn, SIGNAL(currentIndexChanged(int)), this, SLOT(inputChanged(int)));
    connect(ui->leDec, SIGNAL(textChanged(QString)), this, SLOT(decChanged(QString)));
    connect(ui->leHex, SIGNAL(textChanged(QString)), this, SLOT(hexChanged(QString)));
    connect(ui->leBin, SIGNAL(textChanged(QString)), this, SLOT(binChanged(QString)));

    model = new BinModel(this);
    ui->tvBin->setModel(model);
}

NumbersBar::~NumbersBar()
{
    delete ui;
}

void NumbersBar::changeEvent(QEvent *e)
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

void NumbersBar::block(bool b)
{
  ui->leDec->blockSignals(b);
  ui->leHex->blockSignals(b);
  ui->leBin->blockSignals(b);
  ui->tvBin->blockSignals(b);
}

void NumbersBar::setupNumber(int sys)
{
  block(true);

  ui->leDec->setStyleSheet("");
  ui->leHex->setStyleSheet("");
  ui->leBin->setStyleSheet("");

  // count byte number
  int bytes = 8;
  qulonglong tmp = mInput;
  unsigned char c;
  //qDebug() << "---------";
  for (int i = 0; i < 8; i++)
  {
    c = (tmp & (qulonglong)0xff00000000000000l) >> 7*8;
    if (c != 0 && mInput >= 0) break;
    if (c != 0xff && mInput < 0) break;
    tmp <<= 8;
    bytes--;
  }
  if (bytes == 0) bytes = 1;
  mBytes = bytes;

  int bytesIn = ui->cbSizeIn->itemData(ui->cbSizeIn->currentIndex()).toInt() / 8;
  if (bytesIn && bytes > bytesIn)
    ui->cbSizeIn->setStyleSheet(BAD_NUMBER_SHEET);
  else
    ui->cbSizeIn->setStyleSheet("");

  // dec
  if (sys != 10)
  {
    ui->leDec->setText(QString::number(mInput));
  }

  // hex
  if (sys != 16)
  {
    QString num = QString::number(mInput, 16);

    // complete the number
    if (bytesIn && bytes <= bytesIn)
    {
      if (mInput >= 0)
      {
        for (int i = 0; i < (bytesIn-bytes); i++)
            num = "00" + num;
        if (num.length() & 1)
          num = "0" + num;
      }
      else
      {
        num = num.right(bytesIn*2);
      }
    }

    ui->leHex->setText(num);
  }

  // bin
  if (sys != 2)
  {
    QString num = QString::number(mInput, 2);

    // complete the number
    if (bytesIn && bytes <= bytesIn)
    {
      if (mInput >= 0)
      {
        for (int i = 0; i < (bytesIn-bytes); i++)
            num = "00000000" + num;
        if (num.length() % 8)
          num = QString().fill('0', 8 - num.length() % 8) + num;
      }
      else
      {
        num = num.right(bytesIn*8);
      }
    }

    ui->leBin->setText(num);
  }

  model->myBits = ui->leBin->text().length();
  model->myValue = mInput;
  model->reset();

  block(false);
}

void NumbersBar::decChanged(QString text)
{
  mInput = text.toULongLong(&mInputValid, 10);

  if (mInputValid)
  {
    setupNumber(10);
  }
  else
  {
    ui->leDec->setStyleSheet(BAD_NUMBER_SHEET);
  }
}

void NumbersBar::hexChanged(QString text)
{
  if (text.startsWith("0x") || text.startsWith("0X"))
    text = text.right(text.length()-2);

  mInput = text.toULongLong(&mInputValid, 16);

  if (mInputValid)
  {
    setupNumber(16);
  }
  else
  {
    ui->leHex->setStyleSheet(BAD_NUMBER_SHEET);
  }
}

void NumbersBar::binChanged(QString text)
{
  mInput = text.toULongLong(&mInputValid, 2);

  if (mInputValid)
  {
    setupNumber(2);
  }
  else
  {
    ui->leBin->setStyleSheet(BAD_NUMBER_SHEET);
  }
}

void NumbersBar::on_tbOrder_clicked()
{
  int bytes = ui->cbSizeIn->itemData(ui->cbSizeIn->currentIndex()).toInt() / 8;
  if (bytes == 0) bytes = mBytes;

  qulonglong tmp1 = mInput, tmp2 = 0;
  for (int i = 0; i < bytes; i++)
  {
    tmp2 <<= 8;
    tmp2 |= (tmp1 & 0xff);
    tmp1 >>= 8;
  }

  mInput = tmp2;
  setupNumber(0);
}

void NumbersBar::on_tbDecIn_clicked()
{
  QString number;
  emit getNumber(number);
  if (number.isEmpty())
    return;
  ui->leDec->setText(number);
  ui->leDec->setFocus();
}

void NumbersBar::on_tbDecOut_clicked()
{
  emit pasteNumber(ui->leDec->text());
}

void NumbersBar::on_tbHexIn_clicked()
{
  QString number;
  emit getNumber(number);
  if (number.isEmpty())
    return;
  ui->leHex->setText(number);
  ui->leHex->setFocus();
}

void NumbersBar::on_tbHexOut_clicked()
{
  emit pasteNumber(ui->leHex->text());
}

void NumbersBar::on_tbBinIn_clicked()
{
  QString number;
  emit getNumber(number);
  if (number.isEmpty())
    return;
  ui->leBin->setText(number);
  ui->leBin->setFocus();
}

void NumbersBar::on_tbBinOut_clicked()
{
  emit pasteNumber(ui->leBin->text());
}

void NumbersBar::setNumber(const QString &number)
{
  int sys = ui->cbTypeIn->itemData(ui->cbTypeIn->currentIndex()).toInt();
  switch (sys)
  {
  case 10:
    ui->leDec->setText(number);
    ui->leDec->setFocus();
    return;
  case 2:
    ui->leBin->setText(number);
    ui->leBin->setFocus();
    return;
  case 16:
    ui->leHex->setText(number);
    ui->leHex->setFocus();
    return;
  }

  // set the system automatically
  mInput = number.toULongLong(&mInputValid, 2);
  if (mInputValid) {
    ui->leBin->setText(number);
    ui->leBin->setFocus();
    return;
  }

  mInput = number.toULongLong(&mInputValid, 10);
  if (mInputValid) {
    ui->leDec->setText(number);
    ui->leDec->setFocus();
    return;
  }

  mInput = number.toULongLong(&mInputValid, 16);
  if (mInputValid) {
    ui->leHex->setText(number);
    ui->leHex->setFocus();
    return;
  }

  ui->leBin->setStyleSheet(BAD_NUMBER_SHEET);
  ui->leDec->setStyleSheet(BAD_NUMBER_SHEET);
  ui->leHex->setStyleSheet(BAD_NUMBER_SHEET);
}

void NumbersBar::on_tvBin_clicked ( const QModelIndex & index )
{
  QString txt = ui->leBin->text();
  bool bit = txt.at(index.column()) == '1';
  txt[index.column()] = bit ? '0' : '1';
  ui->leBin->setText(txt);
}
