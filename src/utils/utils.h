#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QtDBus>

class Utils
{
public:
    Utils();
    static void sendNotification(QString icon,QString title,QString body);
};

#endif // UTILS_H
