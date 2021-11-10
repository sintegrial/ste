#ifndef DEFINES_H
#define DEFINES_H

#if defined(Q_WS_WIN) || defined(Q_OS_OS2) // Windows, OS/2
#define RESOURCE_DIR QCoreApplication::applicationDirPath() + "/files/"
#else // Linux etc.
#define RESOURCE_DIR QString("/usr/share/sintegrial/ste/files/")
#endif

#define STE_VERSION_STR "0.4.0"

#endif // DEFINES_H
