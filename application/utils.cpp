#include "utils.h"

#ifdef Q_WS_WIN
#include <windows.h>
#endif

Utils::Utils()
{
}

#ifdef Q_WS_WIN

static wchar_t buf[1024];

int Utils::isFileReadonly(const QString &name)
{
  int i = name.toWCharArray(buf);
  buf[i] = 0;

  DWORD attr = GetFileAttributes(buf);
  if (attr == INVALID_FILE_ATTRIBUTES)
    return -1;

  return !!(attr & FILE_ATTRIBUTE_READONLY);
}

int Utils::setFileReadonly(const QString &name, bool set)
{
  int i = name.toWCharArray(buf);
  buf[i] = 0;

  DWORD attr = GetFileAttributes(buf);
  if (attr == INVALID_FILE_ATTRIBUTES)
    return -1;

  if (set)
    attr |= FILE_ATTRIBUTE_READONLY;
  else
    attr &= ~FILE_ATTRIBUTE_READONLY;

  bool res = SetFileAttributes(buf, attr);
  if (!res)
    return 0;
  return 1;
}

QString Utils::normalizeFilename(const QString &name)
{
  return QString(name).replace("/", "\\");
}

QString Utils::driveName(const QString &name)
{
  if (name.isEmpty())
    return "c:\\";

  return QString(name.at(0)) + ":\\";
}

#endif


#ifdef Q_WS_X11

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int Utils::isFileReadonly(const QString &name)
{
  struct stat st;
  int result = stat(name.toAscii().data(), &st);
  if (result)
    return -1;
  if ((st.st_mode & S_IRUSR) == false)
    return 1;
  return 0;
}

int Utils::setFileReadonly(const QString &name, bool set)
{
  int result = chmod(name.toAscii().data(), set ? S_IREAD : S_IREAD | S_IWRITE);
  return !result;
}

QString Utils::normalizeFilename(const QString &name)
{
  return QString(name).replace("\\", "/");
}

QString Utils::driveName(const QString &name)
{
  return "/";
}

#endif
