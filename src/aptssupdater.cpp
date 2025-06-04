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

    // 创建临时文件
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    if (tempFile.open()) {
        QTextStream stream(&tempFile);
        
        for (const QString &line : lines) {
            QRegularExpression regex(R"(([\w\-\+\.]+)/\S+\s+([^\s]+)\s+\S+\s+\[upgradable from: ([^\]]+)\])");
            QRegularExpressionMatch match = regex.match(line);
            if (match.hasMatch()) {
                QString name = match.captured(1);
                QString newVersion = match.captured(2);
                QString oldVersion = match.captured(3);
                
                // 写入内存列表
                packageDetails << QString("%1: %2 → %3").arg(name, oldVersion, newVersion);
                
                // 写入临时文件（原始数据）
                stream << name << "|" << oldVersion << "|" << newVersion << "\n";
            }
        }
        tempFile.close();
        m_tempFilePath = tempFile.fileName();
        qDebug()<< "临时文件路径：" << m_tempFilePath;
        
    } else {
        qWarning() << "无法创建临时文件";
    }

    return packageDetails;
}


QStringList aptssUpdater::getPackageSizes()
{

}

QStringList aptssUpdater::getDesktopAppNames()
{

}