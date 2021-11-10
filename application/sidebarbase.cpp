#include "sidebarbase.h"

////////////////////////////////////////////////////////////////////////////////

SideBarBase::SideBarBase(QWidget *parent) :
    QWidget(parent),
    myFixed(true),
    myStoredHeight(0)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
}

void SideBarBase::showEvent ( QShowEvent * event )
{
  QWidget::showEvent(event);
}

void SideBarBase::hideEvent ( QHideEvent * event )
{
  QWidget::hideEvent(event);
}

void SideBarBase::setupBar(const QString &txt, bool fixed)
{
  QVBoxLayout *vbl = (QVBoxLayout*)layout();
  vbl->setMargin(1);

  QWidget *hdr = new QWidget(this);
  hdr->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  vbl->insertWidget(0, hdr);

  QHBoxLayout *hbl = new QHBoxLayout();
  hdr->setLayout(hbl);
  hbl->setMargin(2);
  hdr->setStyleSheet(".QWidget{border:1px solid #999999; background-color:lightgray;}");

  QLabel *hdrl = new QLabel(txt, this);
  hbl->addWidget(hdrl);

  QToolButton *clos = new QToolButton(this);
  clos->setArrowType(Qt::DownArrow);
  clos->setFixedSize(14,14);
  clos->setShortcut(Qt::Key_Escape);
  connect(clos, SIGNAL(clicked()), this, SIGNAL(closed()));
  hbl->addWidget(clos);

  // fixed?
  myFixed = fixed;
  if (fixed) {
    setFixedHeight(height());
  }
}


void SideBarBase::writeConfig(QSettings &set)
{
  //qDebug() << "writeConfig  " << myStoredHeight;

  set.beginGroup(objectName());

  set.setValue("height", myStoredHeight);

  set.endGroup();
}

void SideBarBase::readConfig(QSettings &set)
{
  set.beginGroup(objectName());

  myStoredHeight = set.value("height", minimumHeight()).toInt();

//  qDebug() << "stored height: " << myStoredHeight;

  set.endGroup();
}
