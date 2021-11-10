#ifndef UTILS_H
#define UTILS_H

#include <QtCore>

class Utils
{
public:
    Utils();

    static int isFileReadonly(const QString &name);
    static int setFileReadonly(const QString &name, bool set);

    static QString normalizeFilename(const QString &name);

    static QString driveName(const QString &name);
};

#endif // UTILS_H
