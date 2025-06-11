#include "downloadmanager.h"
#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QFileInfo> // 添加 QFileInfo 头文件

DownloadManager::DownloadManager(QObject *parent) : QObject(parent) {}

void DownloadManager::startDownload(const QString &url, const QString &outputPath)
{
    QString metalinkUrl = url + ".metalink"; // 构造 Metalink URL
    qDebug() << "开始下载 Metalink 文件:" << metalinkUrl;

    QStringList arguments;
    arguments << "--enable-rpc=false" << "--console-log-level=warn"
              << "--summary-interval=1" << "--dir=" + QFileInfo(outputPath).absolutePath()
              << "--out=" + QFileInfo(outputPath).fileName() << metalinkUrl;

    connect(&m_aria2Process, &QProcess::readyReadStandardOutput, this, &DownloadManager::onAria2Progress);
    connect(&m_aria2Process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &DownloadManager::onAria2Finished);

    m_aria2Process.start("aria2c", arguments);
}

void DownloadManager::onAria2Progress()
{
    QString output = m_aria2Process.readAllStandardOutput();
    QRegularExpression regex(R"(\((\d+)%\))");
    QRegularExpressionMatchIterator i = regex.globalMatch(output);

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.hasMatch()) {
            int progress = match.captured(1).toInt();
            emit downloadProgress(progress); // 发送进度信号
            qDebug() << "下载进度:" << progress << "%";
        }
    }
}


void DownloadManager::onAria2Finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        qDebug() << "下载完成";
        emit downloadFinished(true); // 发送完成信号
    } else {
        qWarning() << "下载失败，退出代码:" << exitCode;
        emit downloadFinished(false); // 发送失败信号
    }
}
