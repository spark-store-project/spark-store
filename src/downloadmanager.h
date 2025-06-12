#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr);
    void startDownload(const QString &packageName, const QString &url, const QString &outputPath); // 修改参数列表
    void cancelDownload(const QString &packageName); // 移动到public区域

signals:
    void downloadProgress(const QString &packageName, int progress);
    void downloadFinished(const QString &packageName, bool success);

private:
    QHash<QString, QProcess*> m_processes;  // 移除旧的m_aria2Process
    // 移除旧的onAria2Progress和onAria2Finished声明
};

#endif // DOWNLOADMANAGER_H
