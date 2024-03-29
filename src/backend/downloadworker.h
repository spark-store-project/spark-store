#ifndef DOWNLOADWORKER_H
#define DOWNLOADWORKER_H

#include <QObject>
#include <QVector>
#include <QProcess>

class DownloadController : public QObject
{
    Q_OBJECT

public:
    explicit DownloadController(QObject *parent = nullptr);

    void setFilename(QString filename);
    void startDownload(const QString &url);
    void stopDownload();
    void restartDownload(QProcess &cmd, const QStringList &command);
    qint64 getFileSize(const QString& url);
    QString replaceDomain(const QString& url, const QString domain);

signals:
    void errorOccur(const QString& msg);
    void downloadProcess(QString, qint64, qint64);
    void downloadFinished();

private:
    int threadNum;
    qint64 pidNumber = -1;
    QString filename;
    qint64 fileSize;
    QVector<QPair<qint64, qint64>> ranges;
    bool finished = false;
    QVector<QString> domains;
};

#endif // FILEDOWNLOADWORKER_H
