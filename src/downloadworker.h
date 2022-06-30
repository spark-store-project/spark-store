#ifndef DOWNLOADWORKER_H
#define DOWNLOADWORKER_H

#include <QObject>
#include <QList>
#include <QFile>
#include <QNetworkReply>

class DownloadController : public QObject
{
    Q_OBJECT

public:
    explicit DownloadController(QObject *parent = nullptr);

    void setFilename(QString filename);
    void startDownload(const QString &url);
    void stopDownload();
    qint64 getFileSize(const QString& url);
    QString replaceDomain(const QString& url, const QString domain);

private:
    int threadNum;
    int pidNumber;
    QString filename;
    qint64 fileSize;
    QVector<QPair<qint64, qint64>> ranges;
    QFile *file;
    bool finished = false;
    QVector<QString> domains;



signals:
    void errorOccur(const QString& msg);
    void downloadProcess(QString, qint64, qint64);
    void downloadFinished();

};

#endif // FILEDOWNLOADWORKER_H
