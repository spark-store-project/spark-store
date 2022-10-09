#include "workerthreads.h"

#include <QProcess>
#include <QDir>
#include <QFile>
#include <QJsonDocument>

#include "widget.h"
#include "HttpClient.h"

void SpkAppInfoLoaderThread::run()
{
    emit requestResetUi();

    httpClient = new AeaQt::HttpClient;
    QString oriSeverUrl = "https://d.store.deepinos.org.cn";
    QString cdnSeverUrl = "https://cdn.d.store.deepinos.org.cn";

    QString downloadTimesUrl = targetUrl.toString();
    downloadTimesUrl = downloadTimesUrl.replace(oriSeverUrl, cdnSeverUrl);
    downloadTimesUrl = downloadTimesUrl.replace("app.json", "download-times.txt");
    httpClient->get(downloadTimesUrl)
            .onResponse([this](QString downloadTimesFeedback)
    {
        qDebug() << "请求应用下载量信息 " << downloadTimesFeedback;
        this->downloadTimes = downloadTimesFeedback.replace("\n","");
    })
    .onError([this](QString errorStr)
    {
        qDebug() << "请求下载量失败:" << errorStr;
        this->downloadTimes  = "0";
    })
    .block()
            .timeout(3*1000)
            .exec();


    httpClient->get(targetUrl.toString().replace(oriSeverUrl, cdnSeverUrl))
            .header("content-type", "application/json")
            .onResponse([this](QByteArray json_array)
    {
        qDebug() << "请求应用信息 " << json_array;
        QString urladdress, deatils, more, packagename, appweb;
        bool isInstalled;
        bool isUpdated;

        // 将路径转化为相应源的下载路径
        urladdress = targetUrl.toString().left(targetUrl.toString().length() - 8);
        QStringList downloadurl = urladdress.split("/");

        QString deburl = serverUrl;
        deburl = deburl.left(urladdress.length() - 1);
        
        QStringList url_ = targetUrl.toString().replace("//", "/").split("/");
        urladdress = "https://" + url_[1];
        // 使用 cdn 服务器
        urladdress = "https://cdn.d.store.deepinos.org.cn";  // 使用图片专用服务器请保留这行，删除后将使用源服务器


        for(int i = 3; i < downloadurl.size(); i++)
        {
            urladdress += "/" + downloadurl[i];
            deburl += "/" + downloadurl[i];
        }

        // 路径转化完成
        QJsonObject json = QJsonDocument::fromJson(json_array).object();
        QString appName = json["Name"].toString();
        QUrl fileUrl = deburl + json["Filename"].toString();

        // 软件信息加载
        QString details;
        details = tr("PkgName: ") + json["Pkgname"].toString() + "\n";
        details += tr("Version: ") + json["Version"].toString() + "\n";
        if(!json["Author"].toString().trimmed().isEmpty())
        {
            details += tr("Author: ") + json["Author"].toString() + "\n";
        }

        if(!json["Website"].toString().trimmed().isEmpty())
        {
            details += tr("Official Site: ") + json["Website"].toString() + "\n";
            // ui->pushButton_website->show();  // move to setinfo slot
            appweb = json["Website"].toString();
        }
        details += tr("Contributor: ") + json["Contributor"].toString() + "\n";
        details += tr("Update Time: ") + json["Update"].toString() + "\n";
        details += tr("Installed Size: ") + json["Size"].toString() + "\n\n";
        details += tr("Download Times: ") + this->downloadTimes + "\n";
        more = json["More"].toString();

        QProcess isInstall;
        packagename = json["Pkgname"].toString();
        isInstall.start("dpkg -s " + json["Pkgname"].toString());
        isInstall.waitForFinished(180); // 默认超时 3 分钟
        int error = QString::fromStdString(isInstall.readAllStandardError().toStdString()).length();
        if(error == 0)
        {
            isInstalled = true;

            QProcess isUpdate;
            isUpdate.start("dpkg-query --showformat='${Version}' --show " + json["Pkgname"].toString());
            isUpdate.waitForFinished(180); // 默认超时 3 分钟
            QString localVersion = isUpdate.readAllStandardOutput();
            localVersion.replace("'", "");

            isUpdate.start("dpkg --compare-versions " + localVersion + " ge " + json["Version"].toString());
            isUpdate.waitForFinished(180); // 默认超时 3 分钟
            if(!isUpdate.exitCode())
            {
                isUpdated = true;
            }
            else
            {
                isUpdated = false;
            }
        }
        else
        {
            isInstalled = false;
            isUpdated = false;
        }

        emit requestSetAppInformation(&appName, &details, &more, &appweb, &packagename, &fileUrl, isInstalled, isUpdated);

        // tag 加载
        QString tags = json["Tags"].toString();
        QStringList tagList = tags.split(";");
        emit requestSetTags(&tagList);

        // 图标加载
        httpClient->get(urladdress+"icon.png")
                .onResponse([this](QByteArray imgData)
        {
            QPixmap appicon;
            appicon.loadFromData(imgData);
            emit finishedIconLoad(&appicon);

            QFile icon("/tmp/spark-store/icon.png");
            icon.open(QFile::WriteOnly);
            appicon.save(&icon, "PNG");
            icon.close();
        })
        .onError([this](QString errorStr)
        {
            Q_UNUSED(this)
            Q_UNUSED(errorStr)

            Widget::sendNotification(tr("Failed to load application icon."));
        })
        .block()
                .timeout(-1)
                .exec();


        // 截图展示加载
        QPixmap screenshotCache[5];
        for(int i = 0; i < 5; i++)
        {
            httpClient->get(urladdress + "screen_" + QString::number(i + 1) + ".png")
                    .onResponse([this, i, &screenshotCache](QByteArray imgData)
            {
                bool s = screenshotCache[i].loadFromData(imgData);
                if(s)
                {
                    emit finishedScreenshotLoad(&screenshotCache[i], i);
                }
                else
                {
                    emit finishedScreenshotLoad(nullptr, i);
                }
            })
                    .onError([this](QString errorStr)
            {
                Q_UNUSED(this)
                Q_UNUSED(errorStr)

                qDebug() << "截图下载失败";
                // Widget::sendNotification(tr("Failed to load application screenshot."));
            })
            .block()
                    .timeout(-1)
                    .exec();
        }
        emit finishAllLoading();

        httpClient->deleteLater();
    })
    .onError([](QString errorStr)
    {
        Q_UNUSED(errorStr)

        Widget::sendNotification(tr("Failed to download app info. Please check internet connection."));
    })
    .block()
            .timeout(-1)
            .exec();
}


void SpkAppInfoLoaderThread::setUrl(const QUrl &url)
{
    targetUrl = url;
}

void SpkAppInfoLoaderThread::setServer(const QString &server)
{
    serverUrl = server;
}

void SpkAppInfoLoaderThread::downloadFinished(int exitcode, QProcess::ExitStatus status)
{
    Q_UNUSED(exitcode)
    Q_UNUSED(status)

    qDebug() << "Finish one download";
    finishedDownload = true;
}

int SpkAppInfoLoaderThread::waitDownload(QProcess& downloader)
{
    while(!downloader.waitForFinished(100))
    {
        if(downloader.state() == QProcess::NotRunning)
        {
            return -1;
        }

        if(this->isInterruptionRequested())
        {
            downloader.terminate();
            downloader.waitForFinished(-1);
            qDebug() << "Appinfo loader thread was forcefully terminated";
            return -1;
        }
    }
    return 0;
}
