#include "dbussparkstore.h"
#include <QDebug>
DBusSparkStoreService::DBusSparkStoreService(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{

}

void DBusSparkStoreService::activeWindow(const QString & arg)
{
    emit sigOpenUrl(arg);
}
