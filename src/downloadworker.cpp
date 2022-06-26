#include "downloadworker.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QThread>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>

DownloadWorker::DownloadWorker(QObject *parent)
{
    Q_UNUSED(parent)
}

void DownloadWorker::setIdentifier(int identifier)
{
    this->identifier = identifier;
}

void DownloadWorker::setParamter(const QString &url, QPair<qint64, qint64> range, QFile *file)
{
    this->url = url;
    this->startPos = range.first;
    this->endPos = range.second;
    this->file = file;
}

qint64 DownloadWorker::getReceivedPos()
{
    return receivedPos;
}

void DownloadWorker::doWork()
{
    mgr = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setRawHeader("Range", QString("bytes=%1-%2").arg(startPos).arg(endPos).toLocal8Bit());
    reply = mgr->get(request);
    qDebug() << "开始下载数据：" << QString(" %1~%2 -> writePos Start %3").arg(startPos).arg(endPos).arg(receivedPos);

    connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError) > (&QNetworkReply::error),
            [this](QNetworkReply::NetworkError error)
    {
        if(error != QNetworkReply::NoError)
        {
            qDebug() << "出错了：" << reply->errorString();
        }
    });
    connect(reply, &QNetworkReply::finished, mgr, &QNetworkAccessManager::deleteLater);
    connect(reply, &QNetworkReply::readyRead, this, &DownloadWorker::dataReady);
    connect(reply, &QNetworkReply::finished, this, &DownloadWorker::slotFinish);
    connect(reply, &QNetworkReply::downloadProgress, this, &DownloadWorker::handleProcess);
    // connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, &DownloadWorker::doStop);
}

void DownloadWorker::doStop()
{
    if (reply) {
        reply->disconnect();
        reply->aboutToClose();
        reply->deleteLater();
        reply = nullptr;
    }
}

void DownloadWorker::dataReady()
{
    QByteArray data = reply->readAll();
    file->seek(startPos + receivedPos);
    file->write(data);
    receivedPos += data.size();
}

void DownloadWorker::slotFinish()
{
    file->flush();
    qDebug() << "数据块下载完毕：" << QString(" %1~%2 -> writePos Start %3").arg(startPos).arg(endPos).arg(receivedPos);
    emit workFinished();
}

void DownloadWorker::handleProcess(qint64, qint64)
{
    emit this->downloadProcess();
}


DownloadController::DownloadController(QObject *parent)
{
    Q_UNUSED(parent)

    // 初始化默认域名
    domains.clear();
    domains.append("d.store.deepinos.org.cn");

    QFile serverList(QDir::homePath().toUtf8() + "/.config/spark-store/server.list");
    if(serverList.open(QFile::ReadOnly))
    {
        QStringList list = QString(serverList.readAll()).trimmed().split("\n");
        qDebug() << list << list.size();

        for (int i = 0; i < list.size(); i++) {
            if (list.at(i).contains("镜像源 Download only") && i + 1 < list.size()) {
                for (int j = i + 1; j < list.size(); j++) {
                    system("curl -I -s --connect-timeout 5 " + list.at(j).toUtf8() 
                        + "/dcs-repo.gpg-key.asc -w  %{http_code}  |tail -n1 > /tmp/spark-store/cdnStatus.txt");
                    QFile cdnStatus("/tmp/spark-store/cdnStatus.txt");
                    if(cdnStatus.open(QFile::ReadOnly) && QString(cdnStatus.readAll()).toUtf8()=="200"){
                        qDebug() << list.at(j);
                        domains.append(list.at(j));
                    }   
                }
                break;
            }
        }
    }
    qDebug() << domains << domains.size();

    /*
    domains = {
        "d1.store.deepinos.org.cn",
        "d2.store.deepinos.org.cn",
        "d3.store.deepinos.org.cn",
        "d4.store.deepinos.org.cn",
        "d5.store.deepinos.org.cn"
    };
    */
    this->threadNum = domains.size();
}

DownloadController::~DownloadController()
{
    if(workers.size() > 0)
    {
        for(int i = 0; i < workers.size(); i++)
        {
            workers.at(i)->doStop();
            workers.at(i)->disconnect();
            workers.at(i)->deleteLater();
        }
        workers.clear();
    }
}

void DownloadController::setFilename(QString filename)
{
    this->filename = filename;
}

void DownloadController::setThreadNum(int threadNum)
{
    this->threadNum = threadNum;
}

/**
 * @brief 开始下载
 */
void DownloadController::startDownload(const QString &url)
{
    finish = 0;

    // 下载任务等分，计算每个线程的下载数据
    fileSize = getFileSize(url);
    if(fileSize == 0)
    {
        emit errorOccur("文件大小获取失败");
        return; 
    }

    qint64 segmentSize = fileSize / threadNum;
    ranges.resize(threadNum);
    QVector<qint64> receivedBytes;
    receivedBytes.resize(threadNum);
    for(int i = 0; i < threadNum; i++)
    {
        ranges[i].first = i * segmentSize;
        ranges[i].second = i * segmentSize + segmentSize - 1;
        receivedBytes[i] = 0;
    }
    ranges[threadNum - 1].second = fileSize; // 余数部分加入最后一个

    // 打开文件
    QDir tmpdir("/tmp/spark-store");
    file = new QFile;
    file->setFileName(tmpdir.absoluteFilePath(filename));

    if(file->exists())
    {
        file->remove();
    }

    if(!file->open(QIODevice::WriteOnly))
    {
        delete file;
        file = nullptr;
        emit errorOccur(file->errorString());
        return;
    }

    file->resize(fileSize);

    // 创建下载线程
    workers.clear();
    for(int i = 0; i < ranges.size(); i++)
    {
        qDebug() << QString("第%1个下载请求：%2-%3").arg(i).arg(ranges.at(i).first).arg(ranges.at(i).second);
        auto worker = new DownloadWorker(this);
        auto range = ranges.at(i);
        QString chunkUrl = replaceDomain(url, domains.at(i));
        worker->setIdentifier(i);
        worker->setParamter(chunkUrl, range, file);
        workers.append(worker);

        connect(worker, &DownloadWorker::downloadProcess, this, &DownloadController::handleProcess);
        connect(worker, &DownloadWorker::workFinished, this, &DownloadController::chunkDownloadFinish);

        worker->doWork();
    }
}

/**
 * @brief 停止下载
 */
void DownloadController::stopDownload()
{
    for(int i = 0; i < workers.size(); i++)
    {
        workers.at(i)->doStop();
        workers.at(i)->disconnect();
        workers.at(i)->deleteLater();
    }
    workers.clear();
    qDebug() << "文件下载路径：" << QFileInfo(file->fileName()).absoluteFilePath();

    file->flush();
    file->close();
    delete file;
    file = nullptr;
}


void DownloadController::handleProcess()
{
    qint64 bytesReceived = 0;
    for(int i = 0; i < workers.size(); i++)
    {
        bytesReceived += workers.at(i)->getReceivedPos();
    }
    qDebug() << QString("下载进度 %1-%2").arg(bytesReceived).arg(fileSize);
    emit downloadProcess(bytesReceived, fileSize);
}

void DownloadController::chunkDownloadFinish()
{
    finish++;
    qDebug() << QString("已下载了%1块，共%2块！！！").arg(finish).arg(threadNum);
    if(finish == threadNum)
    {
        stopDownload();
        emit downloadFinished();
    }
}

qint64 DownloadController::getFileSize(const QString& url)
{
    QEventLoop event;
    QNetworkAccessManager requestManager;
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply *reply = requestManager.head(request);
    connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError) > (&QNetworkReply::error),
            [this, reply](QNetworkReply::NetworkError error)
    {
        if(error != QNetworkReply::NoError)
        {
            emit errorOccur(reply->errorString());
        }
    });
    connect(reply, &QNetworkReply::finished, &event, &QEventLoop::quit);
    event.exec();

    qint64 fileSize = 0;
    if(reply->rawHeader("Accept-Ranges") == QByteArrayLiteral("bytes")
            && reply->hasRawHeader(QString("Content-Length").toLocal8Bit()))
    {
        fileSize = reply->header(QNetworkRequest::ContentLengthHeader).toUInt();
    }
    qDebug() << "文件大小为：" << fileSize;
    reply->deleteLater();
    return fileSize;
}

QString DownloadController::replaceDomain(const QString& url, const QString domain)
{
    QRegularExpression regex(R"((?:[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?\.)+[a-z0-9][a-z0-9-]{0,61}[a-z0-9])");
    if(regex.match(url).hasMatch())
    {
        return QString(url).replace(regex.match(url).captured(), domain);
    }
    return url;
}
