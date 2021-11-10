#ifndef RECENTFILES_H
#define RECENTFILES_H

#include <QtGui>

class RecentFiles : public QObject
{
Q_OBJECT
public:
    explicit RecentFiles(QWidget *parent = 0);

    inline QMenu* menu() { return recentMenu; }
    inline const QStringList& fileNames() const { return recentActions; }

    inline int maxFileCount() const { return maxCount; }
    void setMaxFileCount(int max);

    void addFile(const QString &filename);
    void removeFile(const QString &filename);

    void readConfig(QSettings &set);
    void writeConfig(QSettings &set);

signals:
    void activated(const QString &filename);

protected slots:
    void chosen(QAction*);

protected:
    void updateMenu();

    int maxCount;
    QMenu *recentMenu;
    QStringList recentActions;
};

#endif // RECENTFILES_H
