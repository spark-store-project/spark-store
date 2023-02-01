#ifndef UTILS_H
#define UTILS_H

#include <QObject>

class Utils
{
public:
    static void sendNotification(const QString &icon, const QString &title, const QString &body);
};

#endif // UTILS_H
