#include <QCoreApplication>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QDirIterator>
#include <QProcess>

#include <unistd.h>

QList<QJsonObject> enumAppInfoList()
{
    QList<QJsonObject> appInfoList;
    QDir apps("/opt/apps");
    auto list = apps.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto &appID : list) {
        auto infoPath = apps.absoluteFilePath(appID + "/info");
        QFile infoFile(infoPath);
        if (!infoFile.open(QIODevice::ReadOnly)) {
            continue;
        }
        auto doc = QJsonDocument::fromJson(infoFile.readAll());
        appInfoList.push_back(doc.object());
    }
    return appInfoList;
}
//这段是去找appid和info，没看懂用来干啥的，在此之后info文件也没再用过
//可能以后版本的实现会用到，等官方加功能再说
void linkDir(const QString &source, const QString &target)
{
    auto ensureTargetDir = [](const QString &targetFile) {
        QFileInfo t(targetFile);
        QDir tDir(t.dir());
        tDir.mkpath(".");
    };

    QDir sourceDir(source);
    QDir targetDir(target);
    QDirIterator iter(source, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        auto sourceFile = iter.next();
        auto targetFile = targetDir.absoluteFilePath(sourceDir.relativeFilePath(sourceFile));

        QFileInfo tfi(targetFile);
        //        if (tfi.isSymLink() && (tfi.canonicalFilePath() == sourceFile)) {
	//这里官方应该是写错了，否则会每触发一次就把所有软链接都删了重新创建一次
        if (tfi.isSymLink() && (tfi.symLinkTarget() == sourceFile)) {
            continue;
        } else {
            QFile::remove(targetFile);
        }

        ensureTargetDir(targetFile);
        auto ret = symlink(sourceFile.toStdString().c_str(), targetFile.toStdString().c_str());
        if (0 != ret) {
            qDebug() << "link failed" << sourceFile << "=>" << targetFile << ret;
        }
    }
}

//reset Dynamic library rpath
void setRpath(const QString &file, const QString &path)
{
    if (!QFileInfo::exists(path))
        return;
    QProcess p;
    auto cmd = "patchelf " + file + " --set-rpath " + path;
    p.start("bash", QStringList {"-c", cmd});
    p.waitForFinished();
}

QString getGlic()
{
    //get arch & glibc
    QProcess p;
    auto cmd = "gcc -dumpmachine";
    p.start("bash", QStringList {"-c", cmd});
    p.waitForFinished();
    return p.readAll();
}

void linkApp(const QJsonObject &app)
{
    auto appID = app.value("appid").toString();
    auto appEntriesDir = QDir("/opt/apps/" + appID + "/entries");
    auto appLibsDir = QDir("/opt/apps/" + appID + "/files/lib");
    auto autoStartDir = QDir(appEntriesDir.absoluteFilePath("autostart"));

    bool autoStart = app.value("permissions").toObject().value("autostart").toBool();
    if (autoStart) {
        linkDir(appEntriesDir.absoluteFilePath("autostart"), "/etc/xdg/autostart");
    }

    // link application
    auto sysShareDir = QDir("/usr/share");
    linkDir(appEntriesDir.absoluteFilePath("applications"), sysShareDir.absoluteFilePath("applications"));
    linkDir(appEntriesDir.absoluteFilePath("icons"), sysShareDir.absoluteFilePath("icons"));
    linkDir(appEntriesDir.absoluteFilePath("mime"), sysShareDir.absoluteFilePath("mime"));
    linkDir(appEntriesDir.absoluteFilePath("glib-2.0"), sysShareDir.absoluteFilePath("glib-2.0"));
    linkDir(appEntriesDir.absoluteFilePath("services"), sysShareDir.absoluteFilePath("dbus-1/services"));
    linkDir(appEntriesDir.absoluteFilePath("GConf"), sysShareDir.absoluteFilePath("GConf"));
    linkDir(appEntriesDir.absoluteFilePath("help"), sysShareDir.absoluteFilePath("help"));
    linkDir(appEntriesDir.absoluteFilePath("locale"), sysShareDir.absoluteFilePath("locale"));
    linkDir(appEntriesDir.absoluteFilePath("fcitx"), sysShareDir.absoluteFilePath("fcitx"));
    linkDir(appEntriesDir.absoluteFilePath("polkit"), sysShareDir.absoluteFilePath("polkit-1"));
    linkDir(appEntriesDir.absoluteFilePath("fonts/conf"), "/etc/fonts/conf.d");
    linkDir(appEntriesDir.absoluteFilePath("fonts/files"), sysShareDir.absoluteFilePath("fonts"));


//原来会导致Gtk相关应用翻译缺失，补足了


    auto pluginDir = QDir(appEntriesDir.absoluteFilePath("plugins"));
    if (pluginDir.exists()) {
        QString arch = getGlic();
        //        if (pluginDir.exists()) {
        //            QDirIterator iter(pluginDir.absolutePath(), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        //            while (iter.hasNext()) {
        //                auto sourceFile = iter.next();
        //                setRpath(sourceFile, appLibsDir.absolutePath()); //set rpath
        //            }
        //        }

        linkDir(appEntriesDir.absoluteFilePath("plugins/fcitx"), "/usr/lib/" + arch + "/fcitx");
        linkDir(appEntriesDir.absoluteFilePath("plugins/browser"), "/usr/lib/mozilla/plugins");
    }
}

void cleanLink()
{
    auto cleanDirBrokenLink = [](const QString &dir) {
        QProcess p;
        auto cmd = "find " + dir + " -xtype l -delete";
        p.start("bash", QStringList {"-c", cmd});
        p.waitForFinished();
    };

    QString arch = getGlic();
    auto sysShareDir = QDir("/usr/share");
    cleanDirBrokenLink(sysShareDir.absoluteFilePath("applications"));
    cleanDirBrokenLink(sysShareDir.absoluteFilePath("icons"));
    cleanDirBrokenLink(sysShareDir.absoluteFilePath("mime/packages"));
    cleanDirBrokenLink(sysShareDir.absoluteFilePath("glib-2.0"));
    cleanDirBrokenLink(sysShareDir.absoluteFilePath("dbus-1/services"));
    cleanDirBrokenLink("/etc/xdg/autostart");
    cleanDirBrokenLink(sysShareDir.absoluteFilePath("fcitx"));
    cleanDirBrokenLink(sysShareDir.absoluteFilePath("help"));
    cleanDirBrokenLink(sysShareDir.absoluteFilePath("locale"));
    cleanDirBrokenLink("/usr/lib/" + arch + "/fcitx");
    cleanDirBrokenLink("/usr/lib/mozilla/plugins");
    cleanDirBrokenLink(sysShareDir.absoluteFilePath("polkit-1/actions"));
    cleanDirBrokenLink(sysShareDir.absoluteFilePath("fonts"));
    cleanDirBrokenLink("/etc/fonts/conf.d");
}

void update()
{
    QProcess p;
    auto cmd = "glib-compile-schemas /usr/share/glib-2.0/schemas/";
    p.start("bash", QStringList {"-c", cmd});
    p.waitForFinished();

    cmd = "update-icon-caches /usr/share/icons/*";
    p.start("bash", QStringList {"-c", cmd});
    p.waitForFinished();

    cmd = "update-desktop-database -q";
    p.start("bash", QStringList {"-c", cmd});
    p.waitForFinished();

    cmd = "update-mime-database -V /usr/share/mime";
    p.start("bash", QStringList {"-c", cmd});
    p.waitForFinished();

    
}



int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    cleanLink();

    for (auto &a : enumAppInfoList()) {
        linkApp(a);
    }
    qInfo()<<"Spark dstore patch (c) The Spark Project 2022-Now. Modified from deepin-app-store-tool";

    // trigger
    update();

    return 0;
}
