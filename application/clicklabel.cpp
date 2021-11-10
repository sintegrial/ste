#include "clicklabel.h"

ClickLabel::ClickLabel(QWidget *parent) :
    QLabel(parent),
    myMenu(0)
{
}

void ClickLabel::mousePressEvent ( QMouseEvent * ev )
{
  emit pressed();

  if (myMenu)
  {
    myMenu->popup(ev->globalPos());
  }
}

void ClickLabel::mouseReleaseEvent ( QMouseEvent * /*ev*/ )
{
  emit released();

  emit clicked();
}

void ClickLabel::mouseDoubleClickEvent( QMouseEvent * /*ev*/ )
{
  emit doubleClicked();
}
