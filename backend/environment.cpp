#include "environment.h"
#include <QDir>
#include <QCoreApplication>

#ifndef Q_OS_WIN
#include <cstdlib>
#else
#include <Windows.h>
#define VARIABLE_BUFFER_SIZE MAX_PATH
#endif

bool Environment::get(const QString& variable, QString &result)
{
    result.clear();

#ifndef Q_OS_WIN
    char* res = getenv(variable.toLatin1().constData());

    if (res != NULL) {
        result = res;
        return true;
    }
#else

    LPTSTR value = new TCHAR[MAX_PATH];

#ifdef UNICODE
    const DWORD value_size = ::GetEnvironmentVariable(variable.toStdWString().c_str(), value, MAX_PATH);
#else
    const DWORD value_size = ::GetEnvironmentVariable(variable.toStdString().c_str(), value, MAX_PATH);
#endif

    if (value_size > 0) {
#ifdef UNICODE
        result = QString::fromWCharArray(value, value_size);
#else
        result = QString::fromLocal8Bit(value, value_size);
#endif
    }

    delete[] value;
#endif

    return !result.isEmpty();
}

QString Environment::getDataDirectory()
{
    QString base;
    bool success = false;

#ifdef Q_OS_WIN
    success = get("APPDATA", base);
#else
    success = get("HOME", base);
#endif

    if (success) {
        base += QDir::separator()
#ifndef Q_OS_WIN
            + '.'
#endif
            + qApp->applicationName();
        return base;
    }

    return QString();
}
