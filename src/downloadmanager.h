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
    void startDownload(const QString &url, const QString &outputPath);

signals:
    void downloadProgress(int progress); // 下载进度信号
    void downloadFinished(bool success); // 下载完成信号

private slots:
    void onAria2Progress(); // 处理 aria2 的进度
    void onAria2Finished(int exitCode, QProcess::ExitStatus exitStatus); // 处理 aria2 完成事件

private:
    QProcess m_aria2Process; // 用于运行 aria2 的进程
};

#endif // DOWNLOADMANAGER_H
