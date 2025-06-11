#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QProcess>
#include <QRegularExpression>
class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr);
    ~DownloadManager();

    void startDownload(const QString &appName);
    bool isRunning() const; // 声明 isRunning 方法
    void killProcess(); // 声明 killProcess 方法

signals:
    void downloadProgress(int progress);
    void downloadFinished(bool success);

private slots:
    void onProcessReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_process;
    QRegularExpression m_progressRegex;
};

#endif // DOWNLOADMANAGER_H
