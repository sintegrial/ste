#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QtCore>

struct ToolInfo
{
  QString name, path, version;
  int numVersion, numVersionMin, numVersionRecommended;
  bool present, builtin;
};

class ToolManager : public QObject
{
Q_OBJECT
public:
    explicit ToolManager(QObject *parent = 0);

    const QList<ToolInfo>& toolsInfo() const { return tools; }
signals:

public slots:

private:
    QString getToolInfo(const QString &name, const QString &exe, ToolInfo &info, QString params);
    void getBuiltinToolInfo(const QString &name, ToolInfo &info);
    void addToolInfo(ToolInfo &info);

    QList<ToolInfo> tools;
};

#endif // TOOLMANAGER_H
