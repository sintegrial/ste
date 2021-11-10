#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QtGui>

class TranslationManager : public QObject
{
Q_OBJECT
public:
    explicit TranslationManager(QObject *parent = 0);

    void translate(const QString &lang);

    const QMap<QString, QString>& languages() const { return langMap; }
    QIcon icon(const QString &lang);

    static TranslationManager* instance() { return instance_; }
signals:

public slots:

private:
    QString langPath;
    QMap<QString, QString> langMap;

    QTranslator qtTranslator;
    QTranslator myappTranslator;

    static TranslationManager *instance_;
};

#endif // TRANSLATIONMANAGER_H
