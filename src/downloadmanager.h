#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr);
    void startDownload(const QString &url, const QString &outputPath);
    void cancelDownload(); // 添加取消下载的方法

signals:
    void downloadProgress(int progress); // 下载进度信号
    void downloadFinished(bool success); // 下载完成信号

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

private:
    QNetworkAccessManager m_networkManager;
    QNetworkReply *m_reply = nullptr;
};

#endif // DOWNLOADMANAGER_H
