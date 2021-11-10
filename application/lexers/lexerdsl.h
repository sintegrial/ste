#ifndef LEXERDSL_H
#define LEXERDSL_H

#include <Qsci/qscilexercustom.h>
#include <Qsci/qsciscintilla.h>

#define DSL_DEFAULT       0
#define DSL_DEFINE        1
#define DSL_DEFINE_VALUE  2
#define DSL_TAG           3
#define DSL_ENTRY         4
#define DSL_BRACE         5
#define DSL_ERROR         6


class LexerDSL : public QsciLexerCustom
{
public:
    enum {
      Default = 0,
      Define = 1,
      DefineValue = 2,
      Keyword = 3,
      Entry = 4,
      Brace = 5,
      Error = 6
    };

    LexerDSL(QObject *parent=0);
    virtual ~LexerDSL() {}

    //! Returns the name of the language.
    virtual const char *language() const { return "dsl"; }

    virtual QString description(int style) const;

    virtual QColor defaultColor(int style) const;

    virtual void styleText(int start, int end);

private:
    inline int getCharAt(int pos) const
    //{ return editor()->SendScintilla(QsciScintilla::SCI_GETCHARAT, pos); }
    { return editor()->DirectCharAt(pos); }

    inline int getLineAt(int pos)
    { return editor()->SendScintilla(QsciScintilla::SCI_LINEFROMPOSITION, pos); }

    inline int getEOLfrom(int line)
    { return editor()->SendScintilla(QsciScintilla::SCI_GETLINEENDPOSITION, line); }

    inline bool isWordChar(int ch) const
    { return (ch && ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n'); }

    inline bool isEOL(int ch) const
    { return (ch == '\r' || ch == '\n'); }
};

#endif // LEXERDSL_H
