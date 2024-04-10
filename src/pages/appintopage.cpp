#include "appintopage.h"
#include "ui_appintopage.h"
#include "backend/sparkapi.h"
#include "widgets/downloadlistwidget.h"
#include "widgets/common/downloaditem.h"
#include "backend/image_show.h"
#include "application.h"
#include "utils/utils.h"
#include "pages/settingspage.h"

#include <QtConcurrent>
#include <QClipboard>
#include <QFile>

#include <DSysInfo>
#include <QAtomicInt>

AppIntoPage::AppIntoPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AppIntoPage)
    , api(new SparkAPI(this))
{
    initUI();
    initConnections();
    m_userAgent = QString("Mozilla/5.0 Spark-Store/" + QString(APP_VERSION) + " (Linux;  " + QSysInfo::prettyProductName().toUtf8() + ";)").toLatin1();
}

AppIntoPage::~AppIntoPage()
{
    delete ui;
}

void AppIntoPage::openUrl(const QUrl &url)
{
    clear();

    //    qDebug() << url;
    spk = url;

    SparkAPI *api1 = new SparkAPI(this);
    connect(api1, &SparkAPI::finishedObject, [=](const QJsonObject &appinfo)
            {
        info = appinfo;

        // 显示基本信息
        ui->title->setText(info["Name"].toString());
        ui->version->setText(info["Version"].toString());
        ui->author->setText(info["Author"].toString());
        ui->d_author->setText(info["Author"].toString());
        ui->d_size->setText(info["Size"].toString());
        ui->d_update->setText(info["Update"].toString());
        ui->d_pkgname->setText(info["Pkgname"].toString());
        ui->d_website->setText("<a href=\"" + info["Website"].toString() + "\">" + tr("Click Open"));
        ui->d_contributor->setText(info["Contributor"].toString());
        ui->label_2->setText(info["More"].toString());

        // 显示 tags
        #if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            QStringList taglist = info["Tags"].toString().split(";", Qt::SkipEmptyParts);
        #else
            QStringList taglist = info["Tags"].toString().split(";", QString::SkipEmptyParts);
        #endif

        setAppinfoTags(taglist);

        // 获取图标和截图
        QString pkgUrlBase = api->getImgServerUrl() + SparkAPI::getArchDir() + url.path();
        // 创建网络请求管理器
        QNetworkAccessManager *iconManager = new QNetworkAccessManager(this);

        // 获取图标
        QNetworkRequest iconRequest;
        iconRequest.setUrl(QUrl(pkgUrlBase + "/icon.png"));
        iconRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
        iconRequest.setHeader(QNetworkRequest::ContentTypeHeader, "charset='utf-8'");

        iconManager->get(iconRequest);
        QObject::connect(iconManager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply)
        {
            QByteArray jpegData = reply->readAll();
            iconpixmap.loadFromData(jpegData);
            iconpixmap = iconpixmap.scaled(210, 200, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            ui->icon->setPixmap(iconpixmap);
            ui->icon->setScaledContents(true);

            iconManager->deleteLater();
            reply->deleteLater();
        });


        for (int i = 0; i < 5 /* 魔法数字，最多五个截图 */; i++)
        {
            QString imgUrl = pkgUrlBase + "/screen_" + QString::number(i + 1) + ".png";
            QNetworkRequest request;
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            request.setUrl(QUrl(imgUrl));
            request.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "charset='utf-8'");
            manager->get(request);
            QObject::connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply)
                {
                    QByteArray jpegData = reply->readAll();
                    QPixmap pixmap;
                    if (pixmap.loadFromData(jpegData))
                    {
                        pixmap.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                        image_show *img = new image_show(this);
                        img->setImage(pixmap);
                        // img->setScaledContents(true);
                        QListWidgetItem *pItem = new QListWidgetItem();
                        pItem->setSizeHint(QSize(280, 200));
                        ui->listWidget->insertItem(i, pItem);
                        ui->listWidget->setItemWidget(pItem, img);
                        qDebug() << imgUrl;
                    }

                    manager->deleteLater(); 
                });
        }
        

        // Check UOS
        QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
        if (config.contains("UOS/EnableDeveloperMode") && !config.value("UOS/EnableDeveloperMode").toBool()){
            qDebug() << "UOS Developer Mode has not been enabled!";
            ui->downloadButton->setText(tr("Developer Mode Disabled"));
            ui->downloadButton->setEnabled(false);
            ui->downloadButton->show();
        }
        else // 非 UOS 或 UOS 已经开启开发者模式
        {
            QProcess isInstall;
            bool isInstalled;
            bool isUpdated;
            QString packagename = info["Pkgname"].toString();
            isInstall.start("/opt/durapps/spark-store/bin/store-helper/check-is-installed", QStringList() << info["Pkgname"].toString());
            qDebug() << info["Pkgname"].toString();
            isInstall.waitForFinished(180 * 1000); // 默认超时 3 分钟

            int exitCode = isInstall.exitCode();
            QProcess::ExitStatus exitStatus = isInstall.exitStatus();
            isInstall.close();

            if (exitCode == 0 && exitStatus == QProcess::NormalExit)
            {
                isInstalled = true;

                QProcess isUpdate;
                isUpdate.start("dpkg-query", QStringList() << "--showformat='${Version}'"
                                                           << "--show" << info["Pkgname"].toString());
                isUpdate.waitForFinished(180 * 1000); // 默认超时 3 分钟
                QString localVersion = isUpdate.readAllStandardOutput();
                localVersion.replace("'", "");

                isUpdate.start("dpkg", QStringList() << "--compare-versions" << localVersion << "ge" << info["Version"].toString());
                isUpdate.waitForFinished(180 * 1000); // 默认超时 3 分钟
                if (isUpdate.exitCode() == 0 && isUpdate.exitStatus() == QProcess::NormalExit)
                {
                    isUpdated = true;
                }
                else
                {
                    isUpdated = false;
                }
                isUpdate.close();
            }
            else
            {
                isInstalled = false;
                isUpdated = false;
            }

            if (isInstalled)
            {
                if (isUpdated)
                {
                    ui->downloadButton->setText(tr("Reinstall"));
                    ui->downloadButton->setEnabled(true);
                    ui->downloadButton->show();
                    ui->pushButton_3->show();
                }
                else
                {
                    ui->downloadButton->setText(tr("Upgrade"));
                    ui->downloadButton->setEnabled(true);
                    ui->downloadButton->show();
                    ui->pushButton_3->show();
                }
            }
            else
            {
                ui->downloadButton->setText(tr("Download and Install"));
                ui->downloadButton->setEnabled(true);
                ui->downloadButton->show();
            }

            isDownloading(SparkAPI::getServerUrl() + SparkAPI::getArchDir() + spk.path() + "/" + info["Filename"].toString());
        }

        api1->disconnect();
        api1->deleteLater(); });

    api1->getAppInfo(url);

    api->getAppDownloadTimes(url);
}

void AppIntoPage::clear()
{
    ui->tag_a2d->hide();
    ui->tag_uos->hide();
    ui->tag_dtk5->hide();
    ui->tag_deepin->hide();
    ui->tag_dwine2->hide();
    ui->tag_dwine5->hide();
    ui->tag_ubuntu->hide();
    ui->tag_community->hide();
    ui->icon->clear();
    ui->title->clear();
    ui->version->clear();
    ui->author->clear();
    ui->d_author->clear();
    ui->d_size->clear();
    ui->d_update->clear();
    ui->d_pkgname->clear();
    ui->d_website->clear();
    ui->d_contributor->clear();
    ui->label_2->clear();
    ui->downloadButton->hide();
    ui->downloadButton->setEnabled(false);
    ui->pushButton_3->hide();



    ui->listWidget->clear(); // NOTE: QListWidget::clear() 会析构所有 items
}

void AppIntoPage::setTheme(bool dark)
{
    if (dark)
    {
        QString frameStyleSheet ="#frame,#frame_2,#frame_3,#frame_4 {background-color: #252525; border-radius: 14px; border: 1px solid rgb(64, 64, 64);}\
                                  QLabel#cardtitle,QLabel#title,QLabel#title_1,QLabel#title_2,QLabel#title_3 {color: #FFFFFF}";
        ui->frame->setStyleSheet(frameStyleSheet);
        ui->frame_2->setStyleSheet(frameStyleSheet);
        ui->frame_3->setStyleSheet(frameStyleSheet);
        ui->frame_4->setStyleSheet(frameStyleSheet);

        ui->icon_1->setPixmap(QPixmap(":/icon/dark/box.svg"));
        ui->icon_2->setPixmap(QPixmap(":/icon/dark/box.svg"));
        ui->icon_3->setPixmap(QPixmap(":/icon/dark/calendar.svg"));
        ui->icon_4->setPixmap(QPixmap(":/icon/dark/text.svg"));
        ui->icon_5->setPixmap(QPixmap(":/icon/dark/folder.svg"));
        ui->icon_6->setPixmap(QPixmap(":/icon/dark/globe.svg"));
    }
    else
    {
        //亮色模式
        QString frameStyleSheet ="#frame,#frame_2,#frame_3,#frame_4 {background-color: #fbfbfb; border-radius: 14px; border: 1px solid rgb(229,229,229);}\
                                  QLabel#cardtitle,QLabel#title,QLabel#title_1,QLabel#title_2,QLabel#title_3 {color: #000000}";
        ui->frame->setStyleSheet(frameStyleSheet);
        ui->frame_2->setStyleSheet(frameStyleSheet);
        ui->frame_3->setStyleSheet(frameStyleSheet);
        ui->frame_4->setStyleSheet(frameStyleSheet);

        ui->icon_1->setPixmap(QPixmap(":/icon/light/box.svg"));
        ui->icon_2->setPixmap(QPixmap(":/icon/light/box.svg"));
        ui->icon_3->setPixmap(QPixmap(":/icon/light/calendar.svg"));
        ui->icon_4->setPixmap(QPixmap(":/icon/light/text.svg"));
        ui->icon_5->setPixmap(QPixmap(":/icon/light/folder.svg"));
        ui->icon_6->setPixmap(QPixmap(":/icon/light/globe.svg"));
    }
}

void AppIntoPage::setDownloadWidget(DownloadListWidget *w)
{
    if (dw)
    {
        dw->deleteLater();
        dw = nullptr;
    }

    dw = w;
    connect(w, &DownloadListWidget::downloadFinished, [=]()
    { isDownloading(SparkAPI::getServerUrl() + SparkAPI::getArchDir() + spk.path() + "/" + info["Filename"].toString()); });
}

void AppIntoPage::initUI()
{
    ui->setupUi(this);

    ui->listWidget->setViewMode(QListView::IconMode);
    ui->listWidget->setFlow(QListView::TopToBottom);
    ui->listWidget->setMovement(QListView::Static);
    ui->listWidget->setMaximumHeight(200);

    clear();
}

void AppIntoPage::initConnections()
{
    connect(api, &SparkAPI::finishedRAW, [=](QString download_times)
            {
        download_times = download_times.trimmed();
        qDebug() << "Download Times:" + download_times;
        ui->download_times->setText(download_times); });
}

void AppIntoPage::isDownloading(const QUrl &url)
{
    int index = dw->getUrlList().lastIndexOf(url);
    if (index == -1)
    {
        ui->downloadButton->setEnabled(true);
        return;
    }

    DownloadItem *item = dw->getDIList().at(index);
    if (item == nullptr)
    {
        ui->downloadButton->setEnabled(true);
        return;
    }

    ui->downloadButton->setEnabled(false);
    ui->pushButton_3->hide();
    if (item->download == 2)
    {
        ui->downloadButton->setEnabled(true);
        ui->downloadButton->setText(tr("Download and Install"));
    }
    if (item->download == 1)
    {
        ui->downloadButton->setEnabled(true);
        ui->downloadButton->setText(tr("Install"));
    }
    if (item->isInstall)
    {
        ui->downloadButton->setEnabled(false);
        ui->downloadButton->setText(tr("Installing"));
        return;
    }
    if (item->download == 3)
    {
        QString packageName = info["Pkgname"].toString();
        QProcess process;
        process.start("/opt/durapps/spark-store/bin/store-helper/check-is-installed", {packageName});
        process.waitForFinished(-1);

        int exitCode = process.exitCode();
        QProcess::ExitStatus exitStatus = process.exitStatus();
        process.close();

        if (exitCode == 0 && exitStatus == QProcess::NormalExit)
        {
            ui->downloadButton->setEnabled(true);
            ui->downloadButton->setText(tr("Reinstall"));
            ui->downloadButton->show();
            ui->pushButton_3->show();
        }
        else
        {
            ui->downloadButton->setEnabled(true);
            ui->downloadButton->setText(tr("Download and Install"));
        }
    }
}

void AppIntoPage::setAppinfoTags(const QStringList &tagList)
{
    bool ubuntuSupport = false;
    bool deepinSupport = false;
    bool uosSupport = false;
    foreach (const QString &tag, tagList)
    {
        if (tag == "community")
        {
            ui->tag_community->show();
        }
        else if (tag == "ubuntu")
        {
            ui->tag_ubuntu->show();
            ubuntuSupport = true;
        }
        else if (tag == "deepin")
        {
            ui->tag_deepin->show();
            deepinSupport = true;
        }
        else if (tag == "uos")
        {
            ui->tag_uos->show();
            uosSupport = true;

        }
        else if (tag == "dtk5")
        {
            ui->tag_dtk5->show();
        }
        else if (tag == "dwine2")
        {
            ui->tag_dwine2->show();
        }
        else if (tag == "dwine5")
        {
            ui->tag_dwine5->show();
        }
        else if (tag == "a2d")
        {
            ui->tag_a2d->show();
        }
    }
    notifyUserUnsupportedTags(ubuntuSupport, deepinSupport, uosSupport);
}

void AppIntoPage::notifyUserUnsupportedTags(bool ubuntuSupport, bool deepinSupport, bool uosSupport)
{
    if (!SettingsPage::needUncompatibleNotification) {
        return;
    }

    bool isDeepin = Dtk::Core::DSysInfo::productType() == Dtk::Core::DSysInfo::Deepin;
    bool isUOS = Dtk::Core::DSysInfo::productType() == Dtk::Core::DSysInfo::Uos;
    bool checkdeepin = (isDeepin && !deepinSupport);
    bool checkuos = (isUOS && !uosSupport);
    bool isUbuntu = false;
    if (!checkdeepin && !checkuos)
    {
        // 检查是否为 ubuntu 系统
        QFile lsb("/etc/lsb-release");
        if (!lsb.open(QIODevice::ReadOnly))
        {
            qDebug() << "打开 /etc/lsb-release 失败";
        }
        else if (lsb.readAll().contains("Ubuntu"))
        {
            isUbuntu = true;
            lsb.close();
        }
    }
    bool checkubuntu = (isUbuntu && !ubuntuSupport);

    if (checkdeepin)
    {
        Utils::sendNotification("spark-store", tr("Warning"), tr("The current application does not support deepin, there may be problems"));
    }
    else if (checkuos)
    {
        Utils::sendNotification("spark-store", tr("Warning"), tr("The current application does not support UOS, there may be problems"));
    }
    else if (checkubuntu)
    {
        Utils::sendNotification("spark-store", tr("Warning"), tr("The current application does not support Ubuntu, there may be problems"));
    }
    else if (!isUbuntu && !isDeepin && !isUOS)
    {
        Utils::sendNotification("spark-store", tr("Warning"), tr("The current application does not support current platform, there may be problems"));
    }

    return;
}

void AppIntoPage::on_downloadButton_clicked()
{
    QString downloadUrl = SparkAPI::getServerUrl() + SparkAPI::getArchDir() + spk.path() + "/" + info["Filename"].toString();
    if (ui->downloadButton->text() == tr("Install"))
    {
        DownloadItem *item = dw->getDIList()[dw->getUrlList().lastIndexOf(downloadUrl)];
        if (item == nullptr)
        {
            return;
        }

        connect(item, &DownloadItem::finished, [=]() { isDownloading(downloadUrl); });

        item->install(0);
        isDownloading(downloadUrl);

        return;
    }

    emit clickedDownloadBtn();

    DownloadItem *item = dw->addItem(info["Name"].toString(), info["Filename"].toString(), info["Pkgname"].toString(), iconpixmap, downloadUrl);
    if (item == nullptr)
    {
        return;
    }

    if (ui->downloadButton->text() == tr("Reinstall"))
    {
        item->reinstall = true;
    }
    ui->downloadButton->setEnabled(false);
    connect(item, &DownloadItem::finished, [=]() { isDownloading(downloadUrl); });

    isDownloading(downloadUrl);
}

void AppIntoPage::on_pushButton_3_clicked()
{
    QtConcurrent::run([=]()
    {
        ui->downloadButton->setEnabled(false);
        ui->pushButton_3->setEnabled(false);

        QProcess uninstall;
        uninstall.start("pkexec", QStringList() << "apt" << "autopurge" << "-y" << info["Pkgname"].toString().toLower());
        uninstall.waitForFinished(-1);
        uninstall.close();

        QProcess check;
        check.start("dpkg", QStringList() << "-s" << info["Pkgname"].toString().toLower());
        check.waitForFinished(-1);

        if (check.exitCode() != 0 || check.exitStatus() != QProcess::NormalExit)
        {
            ui->downloadButton->setText(tr("Download and Install"));
            ui->pushButton_3->hide();

            Utils::sendNotification("spark-store",tr("Spark Store"),tr("Uninstall succeeded"));
        }

        ui->downloadButton->setEnabled(true);
        ui->pushButton_3->setEnabled(true);

        check.close();
    });
}

void AppIntoPage::on_shareButton_clicked()
{
    qDebug() << "Share" << spk;
    Application::clipboard()->setText(spk.toString());

    Utils::sendNotification("spark-store", tr("Spark Store"), tr("The URL has been copied to the clipboard"));
}

void AppIntoPage::on_updateButton_clicked()
{
    QString feedbackSpk = "spk://store/chat/store.spark-app.feedback";
    QFileInfo actionSubmissionClientStatus("/opt/durapps/store.spark-app.feedback");
    if (actionSubmissionClientStatus.exists() && actionSubmissionClientStatus.isDir())
    {
        qDebug() << "反馈器存在";
        QProcess::startDetached("sh", QStringList() << "/opt/durapps/store.spark-app.feedback/launch.sh");
    }
    else
    {
        qDebug() << "反馈器不存在，跳转页面";
        openUrl(feedbackSpk);
    }
}

