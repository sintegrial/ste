#ifndef EXTLEXERASM_H
#define EXTLEXERASM_H

#include <Qsci/qscilexerasm.h>

struct ExtLexerASMData
{
  QMap<int, QByteArray> keywords;
};

class ExtLexerASM : public QsciLexerASM
{
Q_OBJECT
public:
    explicit ExtLexerASM(const QString &sid, QObject *parent = 0);
    virtual ~ExtLexerASM();

    static bool registerData(const QString &sid, QMap<QString,QString> &map);

    //! Returns the name of the language.
    const char *language() const { return myLang.constData(); }

    //! Returns the set of keywords for the keyword set \a set recognised by
    //! the lexer as a space separated string.
    const char *keywords(int set) const;

protected:
    QByteArray myLang;
    ExtLexerASMData* myData;

    // map <sid, data>
    static QMap<QString, ExtLexerASMData*> DATA;
};

#endif // EXTLEXERASM_H
