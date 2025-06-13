#include "downloadmanager.h"
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>

DownloadManager::DownloadManager(QObject *parent) : QObject(parent)
{
    cleanupTempFiles();
}

void DownloadManager::startDownload(const QString &packageName, const QString &url, const QString &outputPath)
{
    if (m_processes.contains(packageName)) {
        qWarning() << packageName << " is already downloading.";
        return;
    }

    QString metalinkUrl = url + ".metalink";
    QFileInfo fileInfo(outputPath);

    QStringList arguments = {
        "--enable-rpc=false",
        "--console-log-level=warn",
        "--summary-interval=1",
        "--allow-overwrite=true",
        "--dir=" + fileInfo.absolutePath(),
        "--out=" + fileInfo.fileName(),
        metalinkUrl
    };

    QProcess *process = new QProcess(this);
    m_processes.insert(packageName, process);

    connect(process, &QProcess::readyReadStandardOutput, this, [this, packageName, process]() {
        while (process->canReadLine()) {
            QString line = QString::fromUtf8(process->readLine()).trimmed();
            QRegularExpression regex(R"(\((\d+)%\))");
            QRegularExpressionMatch match = regex.match(line);
            if (match.hasMatch()) {
                int progress = match.captured(1).toInt();
                emit downloadProgress(packageName, progress);
            }
        }
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, packageName, outputPath](int exitCode, QProcess::ExitStatus status) {
        bool success = (exitCode == 0 && status == QProcess::NormalExit);
        if (!success) {
            qWarning() << "Download failed for" << packageName << "exit code:" << exitCode;
        }

        removeAria2Files(outputPath); // 清理残留 .aria2
        emit downloadFinished(packageName, success);

        QProcess *proc = m_processes.take(packageName);
        if (proc) proc->deleteLater();
    });

    process->start("aria2c", arguments);
}

void DownloadManager::cancelDownload(const QString &packageName)
{
    if (!m_processes.contains(packageName)) return;

    QProcess *process = m_processes.take(packageName);
    if (process) {
        process->kill(); // 立即终止
        process->waitForFinished(3000); // 最多等待3秒
        process->deleteLater();
    }

    emit downloadFinished(packageName, false); // 显式通知取消

}

void DownloadManager::removeAria2Files(const QString &filePath)
{
    QString ariaFile = filePath + ".aria2";
    QFile::remove(ariaFile);
}

bool DownloadManager::isDownloading(const QString &packageName) const
{
    return m_processes.contains(packageName);
}

void DownloadManager::cleanupTempFiles()
{
    QDir tempDir(QDir::tempPath());
    QStringList leftovers = tempDir.entryList(QStringList() << "*.aria2", QDir::Files);
    for (const QString &f : leftovers) {
        tempDir.remove(f);
    }
}
