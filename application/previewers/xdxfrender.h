#ifndef XDXFRENDER_H
#define XDXFRENDER_H

#include "previewers/abstractdict.h"

class QsciScintilla;
struct DictViewInfo;
class DictView;

class XDXFRender : public AbstractDict
{
  Q_OBJECT
public:
    XDXFRender(QObject *parent = 0);

private:
    virtual void createIndex();
    virtual void render();
    virtual QString renderString(QString str);
};

#endif // XDXFRENDER_H
