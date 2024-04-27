#include "downloadworker.h"

#include <QProcess>
#include <QRegularExpression>
#include <QtConcurrent>
#include <QStandardPaths>

#define DEFAULTURL "127.0.0.1"
#define MAXWAITTIME 200000

DownloadController::DownloadController(QObject *parent)
{
    Q_UNUSED(parent)

    // 初始化默认域名
    domains.clear();
    domains.append(DEFAULTURL);

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
    QString cmd = QString("curl -I -s --connect-timeout 5 %1 -w  %{http_code}  |tail -n1 > /tmp/spark-store/metaStatus.txt").arg(metaUrl);
    system(cmd.toUtf8().data());
    if (metaStatus.open(QFile::ReadOnly) && QString(metaStatus.readAll()).toUtf8() == "200")
    {
        metaStatus.remove();
        return true;
    }
    return false;
}

void gennerateDomain(QVector<QString> &domains)
{
    QFile serverList(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/server.list");
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
            domains.append(DEFAULTURL);
        }
    }
    else
    {
        domains.append(DEFAULTURL);
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
        if (checkMeatlink(metaUrl))
        {
            useMetalink = true;
            qDebug() << "useMetalink:" << useMetalink;
        }
        else
        {
            gennerateDomain(domains);
            // qDebug() << domains << domains.size();
        }

        QString aria2Command = "-d"; //下载目录
        QString aria2Urls = ""; //下载地址
        QString aria2Verbose = "--summary-interval=1"; //显示下载速度
        QString aria2SizePerThreads = "--min-split-size=1M"; //最小分片大小
        QString aria2NoConfig = "--no-conf"; //不使用配置文件
        QString aria2NoSeeds = "--seed-time=0"; //不做种
        QStringList command;
        QString downloadDir = "/tmp/spark-store/"; //下载目录
        QString aria2ConnectionPerServer = "--max-connection-per-server=1"; //每个服务器最大连接数
        QString aria2ConnectionMax = "--max-concurrent-downloads=16"; //最大同时下载数
        QString aria2ConnectionTimeout = "--connect-timeout=5"; // 5 秒服务器连接超时
        QString aria2MaxRetry = "--max-tries=1" //


        if (useMetalink) //如果是metalink
        {
            command.append(metaUrl.toUtf8());
        }
        else
        {
            for (int i = 0; i < domains.size(); i++) //遍历域名
            {
                command.append(replaceDomain(url, domains.at(i)).replace("+","%2B").toUtf8()); //对+进行转译，避免oss出错
            }
        }


        qint64 downloadSizeRecord = 0; //下载大小记录
        qint8  failDownloadTimes = 0; // 记录重试次数
        const qint8 maxRetryTimes = 3; //最大重试次数
        QString speedInfo = ""; //显示下载速度
        QString percentInfo = ""; //显示下载进度
        command.append(aria2Command.toUtf8());
        command.append(downloadDir.toUtf8());
        command.append(aria2Verbose.toUtf8());
        command.append(aria2NoConfig.toUtf8());
        command.append(aria2SizePerThreads.toUtf8());
        command.append(aria2ConnectionPerServer.toUtf8());
        command.append(aria2ConnectionMax.toUtf8());
        command.append(aria2ConnectionTimeout.toUtf8());
        command.append(aria2MaxRetry.toUtf8());

        if (useMetalink)
        {
            command.append(aria2NoSeeds.toUtf8());
        }
        qDebug() << command;

        bool downloadSuccess = true;
        QProcess cmd;
        cmd.setProcessChannelMode(QProcess::MergedChannels);
        cmd.setProgram("aria2c");
        cmd.setArguments(command);
        cmd.start();
        cmd.waitForStarted(-1); //等待启动完成

        // Timer
        QTimer *timeoutTimer = new QTimer(this);
        timeoutTimer->setSingleShot(true); // 单次触发
        connect(timeoutTimer, &QTimer::timeout, [&]() {
            if (failDownloadTimes < maxRetryTimes) {
                qDebug() << "Download timeout, restarting...";
                // 重新启动下载任务的代码
                restartDownload(cmd, command); // 调用重新启动下载任务的函数
                failDownloadTimes += 1;
                timeoutTimer->start(MAXWAITTIME); // 重新启动定时器
            } else{
                emit errorOccur(tr("Download Failed, please retry :(")); // 下载失败
                downloadSuccess = false;
                cmd.close();
                cmd.terminate(); // 终止当前的下载进程
                cmd.waitForFinished(); // 等待进程结束
            }
        });

        connect(&cmd, &QProcess::readyReadStandardOutput, [&]()
        {
            timeoutTimer->start(MAXWAITTIME); // 重置超时计时器，15秒超时
            //通过读取输出计算下载速度
            QString message = cmd.readAllStandardOutput().data();
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
            if (downloadSize >= downloadSizeRecord)
            {
                downloadSizeRecord = downloadSize;
                timeoutTimer->stop(); // 如果有进度，停止超时计时器
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
        connect(&cmd, &QProcess::readyReadStandardError, [&]()
        {
            emit errorOccur(cmd.readAllStandardError().data());
            downloadSuccess = false;
            cmd.close();
        });

        pidNumber = cmd.processId();

        cmd.waitForFinished(-1);
        cmd.close();

        if(!downloadSuccess)
        {
            return;
        }

        // 统计下载量
        QString SenderdPath = "/opt/durapps/spark-store/bin/ss-feedback/sender-d";
        /*
        * https://en.wikipedia.org/wiki/HD_70642
        * HD 70642 is a star with an exoplanetary companion in the southern constellation of Puppis. 
        */
        QProcess mailProcess;
        mailProcess.start(SenderdPath.toUtf8(), QStringList() << metaUrl << "HD70642");
        mailProcess.waitForStarted();
        mailProcess.waitForFinished(3000);
        mailProcess.close();

        emit downloadFinished(); });
}

/**
 * @brief 停止下载
 */
void DownloadController::stopDownload()
{
    if (pidNumber < 0)
    {
        return;
    }

    // 实现下载进程退出
    QString killCmd = QString("kill -9 %1").arg(pidNumber);
    system(killCmd.toUtf8());
    qDebug() << "kill aria2!";
    pidNumber = -1;
}

void DownloadController::restartDownload(QProcess &cmd, const QStringList &command)
{
    cmd.terminate(); // 终止当前的下载进程
    cmd.waitForFinished(); // 等待进程结束
    cmd.setArguments(command); // 重新设置参数
    cmd.start(); // 重新启动下载
    cmd.waitForStarted(-1); // 等待启动完成
}

qint64 DownloadController::getFileSize(const QString &url)
{
    // 已经无需使用 qtnetwork 再获取 filesize，完全交给 aria2 来计算进度。 为保证兼容性，故保留此函数。
    qDebug() << "Begin download:" << url;
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
