#include "ThemeChecker.h"
#include <QTimer>
#include <QProcess>
#include <QDebug>

ThemeChecker::ThemeChecker(QObject *parent) : QObject(parent), lastColorSchema(-1) {
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkThemeChange()));
    timer->start(1000);
}

void ThemeChecker::checkThemeChange() {


    QProcess process;
    QStringList parameters;
    parameters << "-c" << "dbus-send --session --print-reply=literal --reply-timeout=1000 --dest=org.freedesktop.portal.Desktop /org/freedesktop/portal/desktop org.freedesktop.portal.Settings.Read string:'org.freedesktop.appearance' string:'color-scheme'";
    process.start("/bin/sh", parameters);

    if (process.waitForFinished(-1)) {
        QString errorOutput = QString(process.readAllStandardError()).trimmed();
        if (!errorOutput.isEmpty()) {
            qWarning() << "检测到DBus错误:" << errorOutput;
            return;
        }

        QString output = QString(process.readAll()).trimmed();
        int systemColorSchema = output.right(1).toInt();

        if(systemColorSchema != lastColorSchema) {
            lastColorSchema = systemColorSchema;
            qDebug() << "主题已更改，新的主题是：" << systemColorSchema;
            if(systemColorSchema == 1){
                emit themeChanged(true);
            }
            else if(systemColorSchema == 0 || systemColorSchema == 2){
                emit themeChanged(false);
            }
        }
    } else {
        qWarning() << "DBus调用未能完成";
    }
}

