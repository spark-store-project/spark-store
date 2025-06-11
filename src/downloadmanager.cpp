#include "downloadmanager.h"
#include <QFile>
#include <QDebug>

DownloadManager::DownloadManager(QObject *parent) : QObject(parent) {}

void DownloadManager::startDownload(const QString &url, const QString &outputPath)
{
    qDebug() << "接收到的下载 URL:" << url; // 检查 URL 是否正确传递

    // 验证 URL 是否包含协议
    QUrl metalinkUrl(url + ".metalink");
    if (!metalinkUrl.isValid() || metalinkUrl.scheme().isEmpty()) {
        qWarning() << "无效的 URL:" << metalinkUrl.toString();
        emit downloadFinished(false);
        return;
    }

    QNetworkRequest request(metalinkUrl);
    m_reply = m_networkManager.get(request);

    connect(m_reply, &QNetworkReply::downloadProgress, this, &DownloadManager::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &DownloadManager::onDownloadFinished);

    // 保存路径
    m_reply->setProperty("outputPath", outputPath);
}

void DownloadManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int progress = static_cast<int>((bytesReceived * 100) / bytesTotal);
        emit downloadProgress(progress);
    }
}

void DownloadManager::onDownloadFinished()
{
    if (m_reply->error() == QNetworkReply::NoError) {
        QString outputPath = m_reply->property("outputPath").toString();
        QFile file(outputPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(m_reply->readAll());
            file.close();
            qDebug() << "文件已保存到路径:" << outputPath; // 输出保存路径
            emit downloadFinished(true);
        } else {
            qWarning() << "无法保存文件到路径:" << outputPath;
            emit downloadFinished(false);
        }
    } else {
        qWarning() << "下载失败:" << m_reply->errorString();
        emit downloadFinished(false);
    }

    m_reply->deleteLater();
    m_reply = nullptr;
}

void DownloadManager::cancelDownload()
{
    if (m_reply) {
        m_reply->abort(); // 取消当前下载
        m_reply->deleteLater();
        m_reply = nullptr;
        emit downloadFinished(false); // 发送下载失败信号
    }
}
