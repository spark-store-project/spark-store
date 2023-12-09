#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QJsonObject>

class Utils
{
public:
    static void sendNotification(const QString &icon, const QString &title, const QString &body);
    static bool isDDE();
    static bool isWayland();
    static void initConfig();
    static bool isUOS();
    static bool isPhytium();
    static QString initUUID();
    static void setQPAPlatform();
    static void checkUOSDeveloperMode();
    static QJsonObject parseFeatureJsonFile();
};

#endif // UTILS_H
