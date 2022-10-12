#include "downloadworker.h"
#include <QEventLoop>
#include <QProcess>
#include <QRegularExpression>
#include <QDir>
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


bool checkMeatlink(QString metaUrl)
{
    QFile metaStatus("/tmp/spark-store/metaStatus.txt");
    if (metaStatus.exists())
    {
        metaStatus.remove();
    }
    system("curl -I -s --connect-timeout 5 " + metaUrl.toUtf8() + " -w  %{http_code}  |tail -n1 > /tmp/spark-store/metaStatus.txt");
    if (metaStatus.open(QFile::ReadOnly) && QString(metaStatus.readAll()).toUtf8() == "200")
    {
        metaStatus.remove();
        return true;
    }
    return false;
}

void gennerateDomain(QVector<QString> &domains)
{
    QFile serverList(QDir::homePath().toUtf8() + "/.config/spark-store/server.list");
    if (serverList.open(QFile::ReadOnly))
    {
        QStringList list = QString(serverList.readAll()).trimmed().split("\n");
        qDebug() << list << list.size();
        domains.clear();

        for (int i = 0; i < list.size(); i++)
        {
            if (list.at(i).contains("镜像源 Download only") && i + 1 < list.size())
            {
                for (int j = i + 1; j < list.size(); j++)
                {
                    domains.append(list.at(j));
                }
                break;
            }
        }
        if (domains.size() == 0)
        {
            domains.append("d.store.deepinos.org.cn");
        }
    }
}

/**
 * @brief 开始下载
 */
void DownloadController::startDownload(const QString &url)
{
    // 获取下载任务信息
    fileSize = getFileSize(url);
    if (fileSize == 0)
    {
        emit errorOccur("文件大小获取失败");
        return;
    }

    QtConcurrent::run([=]()
                      {
        QString metaUrl = url + ".metalink";
        qDebug() << "metalink" << metaUrl;
        bool useMetalink = false;
        if (checkMeatlink(metaUrl)){
            useMetalink = true;
            qDebug() << "useMetalink:" << useMetalink;
        }else{
            gennerateDomain(domains);
            // qDebug() << domains << domains.size();
        }

        QString aria2Command = "-d";
        QString aria2Urls = "";
        QString aria2Verbose = "--summary-interval=1";
        QString aria2SizePerThreads = "--min-split-size=1M"; 
        QString aria2NoConfig = "--no-conf";
        QString aria2NoSeeds = "--seed-time=0";
        QStringList command;
        QString downloadDir = "/tmp/spark-store/";
        QString aria2ConnectionPerServer = "--max-connection-per-server=1";

        if (useMetalink){
            command.append(metaUrl.toUtf8());
        }
        else{
            for (int i = 0; i < domains.size(); i++)
            {
                command.append(replaceDomain(url, domains.at(i)).replace("+","%2B").toUtf8()); //对+进行转译，避免oss出错
            }
        }


        qint64 downloadSizeRecord = 0;
        QString speedInfo = "";
        QString percentInfo = "";
        command.append(aria2Command.toUtf8());
        command.append(downloadDir.toUtf8());
        command.append(aria2Verbose.toUtf8());
        command.append(aria2NoConfig.toUtf8());
        command.append(aria2SizePerThreads.toUtf8());
        command.append(aria2ConnectionPerServer.toUtf8());
        if (useMetalink){
            command.append(aria2NoSeeds.toUtf8());
        }
        qDebug() << command;
        auto cmd = new QProcess();
        cmd->setProcessChannelMode(QProcess::MergedChannels);
        cmd->setProgram("aria2c");
        cmd->setArguments(command);
        cmd->start();
        cmd->waitForStarted(); //等待启动完成

        QObject::connect(cmd, &QProcess::readyReadStandardOutput,
        [&]()
        {
            //通过读取输出计算下载速度
            QString message = cmd->readAllStandardOutput().data();
            // qDebug() << message;
            message = message.replace(" ", "");
            QStringList list;
            qint64 downloadSize = 0;
            int downloadSizePlace1 = message.indexOf("(");
            int downloadSizePlace2 = message.indexOf(")");
            int speedPlace1 = message.indexOf("DL:");
            int speedPlace2 = message.indexOf("ETA");
            if (downloadSizePlace1 != -1 && downloadSizePlace2 != -1)
            {
                percentInfo = message.mid(downloadSizePlace1 + 1, downloadSizePlace2 - downloadSizePlace1 - 1).replace("%", "");
                if (percentInfo != "s")
                {
                    int percentInfoNumber = percentInfo.toUInt();

                    downloadSize = percentInfoNumber * fileSize / 100;
                }
            }
            if (speedPlace1 != -1 && speedPlace2 != -1 && speedPlace2 - speedPlace1 <= 15)
            {
                speedInfo = message.mid(speedPlace1 + 3, speedPlace2 - speedPlace1 - 3);
                speedInfo += "/s";
            }
            // qDebug() << percentInfo << speedInfo;
            if (downloadSize >= downloadSizeRecord)
            {
                downloadSizeRecord = downloadSize;
            }
            if (percentInfo == "OK")
            {
                finished = true;
                emit downloadProcess("", fileSize, fileSize);
                qDebug() << "finished:" << finished;
            }
            else
            {
                emit downloadProcess(speedInfo, downloadSizeRecord, fileSize);
            }
        });
        QObject::connect(cmd, &QProcess::readyReadStandardError,
        [&]()
        {
            emit errorOccur(cmd->readAllStandardError().data());
            return;
        });

        auto pidNumber = cmd->processId();
        this->pidNumber = pidNumber;
        while (cmd->waitForFinished(-1))
        {
            continue;
        }

        // 统计下载量
        QString SenderdPath = "/opt/durapps/spark-store/bin/ss-feedback/sender-d";
        /*
        * https://en.wikipedia.org/wiki/HD_70642
        * HD 70642 is a star with an exoplanetary companion in the southern constellation of Puppis. 
        */
        system(SenderdPath.toUtf8() + " " + metaUrl.toUtf8() + " " + "HD70642");

        emit downloadFinished(); });
}

/**
 * @brief 停止下载
 */
void DownloadController::stopDownload()
{
    // 实现下载进程退出
    QString killCmd = QString("kill -9 %1").arg(pidNumber);
    system(killCmd.toUtf8());
    qDebug() << "kill aria2!";
}

qint64 DownloadController::getFileSize(const QString &url)
{
    // 已经无需使用 qtnetwork 再获取 filesize，完全交给 aria2 来计算进度。 为保证兼容性，故保留此函数。
    qint64 fileSize = 10000;
    return fileSize;
}

QString DownloadController::replaceDomain(const QString &url, const QString domain)
{
    QRegularExpression regex(R"((?:[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?\.)+[a-z0-9][a-z0-9-]{0,61}[a-z0-9])");
    if (regex.match(url).hasMatch())
    {
        return QString(url).replace(regex.match(url).captured(), domain);
    }
    return url;
}
