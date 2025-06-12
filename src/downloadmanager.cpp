#include "downloadmanager.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>

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

    QProcess *process = new QProcess(this);
    m_processes.insert(packageName, process);

    // ✅ 改为按行读取（逐行处理）
    connect(process, &QProcess::readyReadStandardOutput, [this, process, packageName]() {
        while (process->canReadLine()) {
            QString line = process->readLine().trimmed();
            QRegularExpression regex(R"(\((\d+)%\))");
            QRegularExpressionMatch match = regex.match(line);
            if (match.hasMatch()) {
                int progress = match.captured(1).toInt();
                emit downloadProgress(packageName, progress);
                qDebug() << "下载进度:" << progress << "%";
            }
        }
    });

    // 下载完成
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
        QProcess *process = m_processes[packageName];
        process->terminate();
        process->waitForFinished();
        m_processes.remove(packageName);
    }
}
