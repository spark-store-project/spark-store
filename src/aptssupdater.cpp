#include "aptssupdater.h"
#include <QProcess>
#include <QTextStream>
#include <QRegularExpression>
aptssUpdater::aptssUpdater(QWidget *parent)
    : QWidget(parent)
{

}
QStringList aptssUpdater::getUpdateablePackages()
{
    QStringList packageDetails;
    QProcess process;
    process.start("bash", QStringList() << "-c" << "aptss list --upgradable");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QTextStream stream(&output);

    // 跳过第一行（提示信息）
    stream.readLine();

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QRegularExpression regex(R"((\S+)/\S+\s+(\S+)\s+\S+\s+\[可从该版本升级：(.+)\])");
        QRegularExpressionMatch match = regex.match(line);

        if (match.hasMatch()) {
            QString packageName = match.captured(1);
            QString currentVersion = match.captured(2);
            QString updateVersion = match.captured(3);
            packageDetails << QString("%1 (%2 -> %3)").arg(packageName, currentVersion, updateVersion);
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