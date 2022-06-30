#include "downloadworker.h"
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QThread>
#include <QProcess>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>
#include <QElapsedTimer>
#include <QtConcurrent>

DownloadController::DownloadController(QObject *parent)
{
    Q_UNUSED(parent)

    // 初始化默认域名
    domains.clear();
    domains.append("d.store.deepinos.org.cn");



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



void DownloadController::setFilename(QString filename)
{
    this->filename = filename;
}


void timeSleeper(int time)
{
    QElapsedTimer t1;
    t1.start();
    while(t1.elapsed()<time);
    return;
}

//int checkPID(QString pidCommand){
//    system(pidCommand.toUtf8());
//    timeSleeper(10);
//    QFile downloadStatus("/tmp/spark-store/downloadStatus.txt");
//    downloadStatus.open(QFile::ReadOnly);
//    auto temp = QString(downloadStatus.readAll()).toUtf8();
//    downloadStatus.close();
//    if (temp!=""){
//        return 1;
//    }
//    return 0;
//}

/**
 * @brief 开始下载
 */
void DownloadController::startDownload(const QString &url)
{
    // 获取下载任务信息
    fileSize = getFileSize(url);
    if(fileSize == 0)
    {
        emit errorOccur("文件大小获取失败");
        return; 
    }


    QtConcurrent::run([=](){
        QFile serverList(QDir::homePath().toUtf8() + "/.config/spark-store/server.list");
        if(serverList.open(QFile::ReadOnly))
        {
            QStringList list = QString(serverList.readAll()).trimmed().split("\n");
            qDebug() << list << list.size();

            for (int i = 0; i < list.size(); i++) {
                if (list.at(i).contains("镜像源 Download only") && i + 1 < list.size()) {
                    for (int j = i + 1; j < list.size(); j++) {
                        system("curl -I -s --connect-timeout 5 https://" + list.at(j).toUtf8()
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
        QDir tmpdir("/tmp/spark-store/");
        QString aria2Command = "-d";
        QString aria2Urls = "";
        QString aria2Verbose = "--summary-interval=1";
        QString aria2Threads = "-s " + QString::number(domains.size());
        QStringList command;
        QString downloadDir = "/tmp/spark-store/";
        for(int i = 0; i < domains.size(); i ++)
        {
            command.append(replaceDomain(url, domains.at(i)).toUtf8());
            aria2Urls += replaceDomain(url, domains.at(i));
            aria2Urls += " ";
        }

        qint64 downloadSizeRecord = 0;
        QString speedInfo = "";
        QString percentInfo = "";
        command.append(aria2Command.toUtf8());
        command.append(downloadDir.toUtf8());
        command.append(aria2Verbose.toUtf8());
        command.append(aria2Threads.toUtf8());
        qDebug() << command;
        auto cmd = new QProcess();
        cmd->setProcessChannelMode(QProcess::MergedChannels);
        cmd->setProgram("aria2c");
        cmd->setArguments(command);
        cmd->start();
        cmd->waitForStarted();            //等待启动完成

        QObject::connect(cmd,&QProcess::readyReadStandardOutput,
                           [&](){
            //通过读取输出计算下载速度
            QFileInfo info(tmpdir.absoluteFilePath(filename));
            QString message = cmd->readAllStandardOutput().data();
            message = message.replace(" ","").replace("\n","").replace("-","");
            message = message.replace("*","").replace("=","");
            QStringList list;
            qint64 downloadSize = 0;
            int downloadSizePlace1 = message.indexOf("(");
            int downloadSizePlace2 = message.indexOf(")");
            int speedPlace1 = message.indexOf("DL:");
            int speedPlace2 = message.indexOf("ETA");
            if (downloadSizePlace1 != -1 && downloadSizePlace2 != -1){
                percentInfo = message.mid(downloadSizePlace1+1, downloadSizePlace2-downloadSizePlace1-1).replace("%","");
                if (percentInfo != "s"){
                    int percentInfoNumber = percentInfo.toUInt();
                    
                    downloadSize = (percentInfoNumber+1) * fileSize / 100;
                }
            }
            if (speedPlace1 != -1 && speedPlace2 != -1){
                speedInfo = message.mid(speedPlace1+3, speedPlace2-speedPlace1-3);
                speedInfo += "/s";

            }
            qDebug() << percentInfo << speedInfo;
            if(downloadSize >= downloadSizeRecord)
            {
                downloadSizeRecord = downloadSize;
            }
            if(percentInfo=="OK"){
                finished = true;
                emit downloadProcess("", fileSize, fileSize);
                qDebug() <<"finished:"<< finished;
            }
            else{
                emit downloadProcess(speedInfo, downloadSizeRecord, fileSize);
            }
            


          });
        QObject::connect(cmd,&QProcess::readyReadStandardError,
                           [&](){
            emit errorOccur(cmd->readAllStandardError().data());
            return;
          });

        auto pidNumber = cmd->processId();
        this->pidNumber = pidNumber;
        int statusSum = 0;
        while(statusSum > -20)
        {
            auto status = cmd->waitForFinished() - 1;
            statusSum += status;
        }
        emit downloadFinished();

    });

}

/**
 * @brief 停止下载
 */
void DownloadController::stopDownload()
{
    // 实现下载进程退出
    QString killCmd = QString("kill -9 %1").arg(pidNumber);
    system(killCmd.toUtf8());
    qDebug()<<"kill aria2!";

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
