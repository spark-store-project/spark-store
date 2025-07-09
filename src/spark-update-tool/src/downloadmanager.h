#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QMap>
#include <QProcess>

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr);
    void startDownload(const QString &packageName, const QString &url, const QString &outputPath);
    void cancelDownload(const QString &packageName);
    bool isDownloading(const QString &packageName) const;

signals:
    void downloadProgress(const QString &packageName, int progress);
    void downloadFinished(const QString &packageName, bool success);

private:
    void cleanupTempFiles();
    void removeAria2Files(const QString &filePath);

    QMap<QString, QProcess*> m_processes;
};

#endif // DOWNLOADMANAGER_H
