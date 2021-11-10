#ifndef SIDEBARBASE_H
#define SIDEBARBASE_H

#include <QtGui>

class SideBarBase : public QWidget
{
  Q_OBJECT

  friend class BarHeader;

  public:
    explicit SideBarBase(QWidget *parent = 0);

    void setupBar(const QString &txt, bool fixed);

    inline bool isFixed() const { return myFixed; }

    inline int storedHeight() const { return myStoredHeight; }
    inline void setStoredHeight(int h) { myStoredHeight = h; }

    virtual void writeConfig(QSettings &set);
    virtual void readConfig(QSettings &set);

  signals:
    void closed();

  protected:
    virtual void showEvent ( QShowEvent * event );
    virtual void hideEvent ( QHideEvent * event );

    bool myFixed;
    int myStoredHeight;
};

#endif // SIDEBARBASE_H
