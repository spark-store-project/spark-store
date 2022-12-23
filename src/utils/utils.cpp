#include "utils.h"

Utils::Utils()
{
}

// Author: chatGPT
void Utils::sendNotification(QString icon, QString title, QString body)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications");

    QVariantList args;
    args << QCoreApplication::applicationName() // the name of the application
         << (uint)0                             // replaces the previous notification with the same ID
         << icon                                // the application icon of the notification
         << title                               // the title of the notification
         << body                                // the body of the notification
         << QStringList()                       // no actions
         << QVariantMap()                       // no hints
         << -1;                                 // no timeout

    iface.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
}
