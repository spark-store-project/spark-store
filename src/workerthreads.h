#ifndef WORKERTHREADS_H
#define WORKERTHREADS_H

#include <QThread>
#include <QPixmap>
#include <QUrl>
#include <QProcess>

namespace AeaQt {
class HttpClient;
}

class SpkAppInfoLoaderThread Q_DECL_FINAL : public QThread
{
    Q_OBJECT

public:
    // explicit SpkAppInfoLoaderThread() = default;

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QUrl targetUrl;
    QString serverUrl;
    bool finishedDownload = false;
    int downloaderRetval = 0;

    AeaQt::HttpClient *httpClient;

    int waitDownload(QProcess& downloader);

public slots:
    void setUrl(const QUrl &url);
    void setServer(const QString &server);
    void downloadFinished(int exitcode, QProcess::ExitStatus status);

signals:
    void requestResetUi();
    void requestSetTags(QStringList *tagList);
    void requestSetAppInformation(QString *name, QString *details, QString *info,
                                  QString *website, QString *packageName,
                                  QUrl *fileUrl, int downloadCount, bool isInstalled,
                                  bool isUpdated);
    void finishedIconLoad(QPixmap *icon);
    void finishedScreenshotLoad(QPixmap *icon, int index);  // 该信号必须以 BlockingQueued 方式连接
    void finishAllLoading();                                // 该信号必须以 BlockingQueued 方式连接

};

#endif // WORKERTHREADS_H
