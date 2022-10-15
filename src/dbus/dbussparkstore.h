#ifndef DBUSSPARKSTORE_H
#define DBUSSPARKSTORE_H

#include <QObject>
#include <QUrl>
#include <QtDBus/QtDBus>

class Wallpaper;
class DBusSparkStoreService : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.gitee.spark.store")
public:
    explicit DBusSparkStoreService(QObject *parent);

signals :
    void sigOpenUrl(const QString &url);
public Q_SLOTS:

    void activeWindow(const QString &arg);

};

#endif // DBUSSPARKSTORE_H
