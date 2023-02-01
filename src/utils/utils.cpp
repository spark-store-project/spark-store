#include "utils.h"
#include "application.h"

#include <QDBusInterface>

// Author: chatGPT
void Utils::sendNotification(const QString &icon, const QString &title, const QString &body)
{
    QDBusInterface interface("org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications");

    QVariantList args;
    args << Application::applicationName() // the name of the application
         << static_cast<quint32>(0)        // replaces the previous notification with the same ID
         << icon                           // the application icon of the notification
         << title                          // the title of the notification
         << body                           // the body of the notification
         << QStringList()                  // no actions
         << QVariantMap()                  // no hints
         << -1;                            // no timeout

    interface.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
}
