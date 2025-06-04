#include "aptssupdater.h"
#include <QProcess>
#include <QTextStream>
#include <QRegularExpression>
#include <QFile>

aptssUpdater::aptssUpdater(QWidget *parent)
    : QWidget(parent)
{

}

QStringList aptssUpdater::getUpdateablePackages()
{
    QStringList packageDetails;
    QProcess process;
    QString command = R"(env LANGUAGE=en_US /usr/bin/apt -c /opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf list --upgradable -o Dir::Etc::sourcelist="/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list" -o Dir::Etc::sourceparts="/dev/null" -o APT::Get::List-Cleanup="0" | awk 'NR>1')";
    
    process.start("bash", QStringList() << "-c" << command);
    if (!process.waitForFinished()) {
        qWarning() << "Process failed to finish.";
        return packageDetails;
    }

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        // 示例行格式:
        // code/unknown 1.100.3-1748872405 amd64 [upgradable from: 1.100.2-1747260578]

        QRegularExpression regex(R"(([\w\-\+\.]+)/\S+\s+([^\s]+)\s+\S+\s+\[upgradable from: ([^\]]+)\])");
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            QString name = match.captured(1);
            QString newVersion = match.captured(2);
            QString oldVersion = match.captured(3);
            packageDetails << QString("%1: %2 → %3").arg(name, oldVersion, newVersion);
        }
    }

    return packageDetails;
}


QStringList aptssUpdater::getPackageSizes()
{
    QStringList packageSizes;
    QProcess process;
    process.start("bash", QStringList() << "-c" << "aptss upgrade --assume-no");

    // 自动回复 "n" 以跳过交互式提示
    if (process.waitForStarted()) {
        process.write("n\n");
    }

    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QTextStream stream(&output);

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QRegularExpression regex(R"((\S+)\s+\S+\s+(\d+(?:\.\d+)?(?:KiB|MiB)))");
        QRegularExpressionMatch match = regex.match(line);

        if (match.hasMatch()) {
            QString packageName = match.captured(1);
            QString packageSize = match.captured(2);
            packageSizes << QString("%1: %2").arg(packageName, packageSize);
        }
    }

    return packageSizes;
}

QStringList aptssUpdater::getDesktopAppNames()
{
    QStringList appNames;
    QStringList updateablePackages = getUpdateablePackages();

    for (const QString &packageDetail : updateablePackages) {
        // 提取包名（忽略版本信息）
        QString packageName = packageDetail.split(' ').first();
        QString desktopFilePath = QString("/usr/share/applications/%1.desktop").arg(packageName);

        QFile desktopFile(desktopFilePath);
        if (desktopFile.exists() && desktopFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&desktopFile);
            while (!stream.atEnd()) {
                QString line = stream.readLine();
                if (line.startsWith("Name=")) {
                    QString appName = line.mid(5); // 提取 Name 属性值
                    appNames << appName;
                    break;
                }
            }
            desktopFile.close();
        } else {
            appNames << QString("未找到 .desktop 文件: %1").arg(packageName);
        }
    }

    return appNames;
}