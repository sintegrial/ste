#ifndef DSLRENDER_H
#define DSLRENDER_H

#include "previewers/abstractdict.h"

class QsciScintilla;

class DSLRender : public AbstractDict
{
  Q_OBJECT
public:
    DSLRender(QObject *parent = 0);

private:
    virtual void createIndex();
    virtual void render();
    virtual QString renderString(QString str);
};

#endif // DSLRENDER_H
