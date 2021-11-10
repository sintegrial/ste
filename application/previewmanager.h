#ifndef PREVIEWMANAGER_H
#define PREVIEWMANAGER_H

#include <QtGui>

class PreviewManagerDialog;
class QsciScintilla;

class PreviewManager : public QObject
{
Q_OBJECT
public:
    explicit PreviewManager(QObject *parent = 0);

    bool preview(QsciScintilla *editor, const QString &sid, const QString &filename);

    // config
    void readConfig(QSettings &set);
    void writeConfig(QSettings &set);

signals:

public slots:

private:
    PreviewManagerDialog *previewDialog;
};

#endif // PREVIEWMANAGER_H
