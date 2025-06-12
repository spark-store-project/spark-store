#include "downloadmanager.h"
#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QFileInfo> // 添加 QFileInfo 头文件

DownloadManager::DownloadManager(QObject *parent) : QObject(parent) {}

void DownloadManager::startDownload(const QString &packageName, const QString &url, const QString &outputPath)
{
    QString metalinkUrl = url + ".metalink";
    QStringList arguments;
    arguments << "--enable-rpc=false" 
              << "--console-log-level=warn"
              << "--summary-interval=1" 
              << "--dir=" + QFileInfo(outputPath).absolutePath()
              << "--out=" + QFileInfo(outputPath).fileName() 
              << metalinkUrl;

    QProcess* process = new QProcess(this);
    m_processes.insert(packageName, process);

    connect(process, &QProcess::readyReadStandardOutput, [this, process, packageName]() {
        QString output = process->readAllStandardOutput();
        QRegularExpression regex(R"(\((\d+)%\))");
        QRegularExpressionMatchIterator i = regex.globalMatch(output);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            if (match.hasMatch()) {
                int progress = match.captured(1).toInt();
                emit downloadProgress(packageName, progress);
                qDebug() << "下载进度:" << progress << "%";
            }
        }
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this, packageName](int exitCode, QProcess::ExitStatus exitStatus) {
            bool success = (exitCode == 0 && exitStatus == QProcess::NormalExit);
            emit downloadFinished(packageName, success);
            m_processes.remove(packageName);
        });

    process->start("aria2c", arguments);
}

void DownloadManager::cancelDownload(const QString &packageName)
{
    if (m_processes.contains(packageName)) {
        QProcess* process = m_processes[packageName];
        process->terminate();
        process->waitForFinished();
        m_processes.remove(packageName);
    }
}
