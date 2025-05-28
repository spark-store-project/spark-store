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
        QRegularExpression regex(R"((\S+)/\S+\s+\S+\s+\S+\s+\[可从该版本升级：(.+)\])");
        QRegularExpressionMatch match = regex.match(line);

        if (match.hasMatch()) {
            QString packageName = match.captured(1);
            QString updateDetails = match.captured(2);
            packageDetails << QString("%1: %2").arg(packageName, updateDetails);
        }
    }

    return packageDetails;
}
