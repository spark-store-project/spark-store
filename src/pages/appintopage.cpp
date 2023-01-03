#include "appintopage.h"
#include "ui_appintopage.h"
AppIntoPage::AppIntoPage(QWidget *parent) : QWidget(parent),
                                            ui(new Ui::AppIntoPage)
{
    ui->setupUi(this);
    ui->listWidget->setViewMode(QListView::IconMode);
    ui->listWidget->setMovement(QListView::Static);
    ui->listWidget->setMaximumHeight(200);
    ui->listWidget->setFlow(QListView::TopToBottom);
    api1 = new SparkAPI(this);
    connect(api1, &SparkAPI::finishedRAW, [=](QString download_times)
            {
        download_times.remove(QRegExp("\\n"));
        ui->download_times->setText(download_times);
        qDebug()<<"Download Times:"+download_times; });
    clear();
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
    ui->author->clear();
    ui->label_2->clear();
    ui->downloadButton->hide();
    ui->downloadButton->setEnabled(false);
    ui->pushButton_3->hide();
    int n = ui->listWidget->count();
    for (int i = 0; i < n; i++)
    {
        QListWidgetItem *item = ui->listWidget->takeItem(0);
        QWidget *card = ui->listWidget->itemWidget(item);
        delete card;
        card = NULL;
        delete item;
        item = NULL;
    }
    ui->listWidget->clear();
}
void AppIntoPage::setDownloadWidget(DownloadListWidget *w)
{
    dw = w;
    connect(w, &DownloadListWidget::downloadFinished, [=]()
            { isDownloading(SparkAPI::getServerUrl() + "store" + spk.path() + "/" + info["Filename"].toString()); });
}
void AppIntoPage::openUrl(QUrl url)
{
    spk = url;
    SparkAPI *api = new SparkAPI(this);
    clear();
    connect(api, &SparkAPI::finishedObject, [=](QJsonObject appinfo)
            {

        info = appinfo;
//        qDebug()<<url;
        //获取图标
        QNetworkRequest request;
        QNetworkAccessManager *naManager = new QNetworkAccessManager(this);
        qDebug()<<api->getImgServerUrl()+"store"+url.path().replace("+","%2B") + "/icon.png";
        request.setUrl(QUrl(api->getImgServerUrl()+"store"+url.path().replace("+","%2B") + "/icon.png"));
        request.setRawHeader("User-Agent", "Mozilla/5.0");
        request.setRawHeader("Content-Type", "charset='utf-8'");
        naManager->get(request);
        QObject::connect(naManager,&QNetworkAccessManager::finished,[=](QNetworkReply *reply){
                QByteArray jpegData = reply->readAll();
                iconpixmap.loadFromData(jpegData);
                iconpixmap.scaled(210, 200, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                ui->icon->setPixmap(iconpixmap);
                ui->icon->setScaledContents(true);
            });
        ui->title->setText(info["Name"].toString());
        ui->version->setText(info["Version"].toString());
        ui->author->setText(info["Author"].toString());
        ui->d_author->setText(info["Author"].toString());
        ui->d_size->setText(info["Size"].toString());
        ui->d_update->setText(info["Update"].toString());
        ui->d_pkgname->setText(info["Pkgname"].toString());
        ui->d_website->setText("<a href=\""+info["Website"].toString()+"\">"+tr("Click Open"));
        ui->d_contributor->setText(info["Contributor"].toString());
        ui->label_2->setText(info["More"].toString());


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
            isInstall.start("dpkg", QStringList() << "-s" << info["Pkgname"].toString());
            qDebug() << info["Pkgname"].toString();
            isInstall.waitForFinished(180 * 1000); // 默认超时 3 分钟
            int error = QString::fromStdString(isInstall.readAllStandardError().toStdString()).length();
            if (error == 0)
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
                if (!isUpdate.exitCode())
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

            QObject::connect(naManager, &QNetworkAccessManager::finished, [=]()
                             {

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
                    ui->downloadButton->setText(tr("Download"));
                    ui->downloadButton->setEnabled(true);
                    isDownloading(SparkAPI::getServerUrl() + "store" + spk.path() + "/" + info["Filename"].toString());
                    ui->downloadButton->show();
                } });
        }

        QStringList taglist = info["Tags"].toString().split(";");
        QString tmp=info["img_urls"].toString();
        qDebug() << tmp;
        if (tmp.left(2) == "[\"")
        {
            tmp.remove(0, 2);
        }
        if (tmp.right(2) == "\"]")
        {
            tmp.remove(tmp.size() - 2, tmp.size());
        }
        QStringList imglist = tmp.split("\",\"");
        qDebug() << imglist;
        for (int i = 0; i < imglist.size(); i++)
        {
            QNetworkRequest request;
            QNetworkAccessManager *iconNaManager = new QNetworkAccessManager(this);
            request.setUrl(QUrl(imglist[i].replace("+", "%2B")));
            request.setRawHeader("User-Agent", "Mozilla/5.0");
            request.setRawHeader("Content-Type", "charset='utf-8'");
            iconNaManager->get(request);
            QObject::connect(iconNaManager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply)
                             {
                    QByteArray jpegData = reply->readAll();
                    QPixmap pixmap;
                    pixmap.loadFromData(jpegData);
                    pixmap.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    image_show *img=new image_show(this);
                    img->setImage(pixmap);
                    //img->setScaledContents(true);
                    QListWidgetItem* pItem = new QListWidgetItem();
                    pItem->setSizeHint(QSize(280, 200));
                    ui->listWidget->addItem(pItem);
                    ui->listWidget->setItemWidget(pItem, img);
                });
        }
        this->sltAppinfoTags(&taglist);
        disconnect(api,&SparkAPI::finished,nullptr,nullptr);
        api->deleteLater(); });
    api->getAppInfo(url);
    api1->getAppDownloadTimes(url);
}

void AppIntoPage::isDownloading(QUrl url)
{
    if (dw->getUrlList().lastIndexOf(url) == -1)
    {
        ui->downloadButton->setEnabled(true);
        return;
    }
    else
    {
        ui->downloadButton->setEnabled(false);
    }

    ui->pushButton_3->hide();
    if (dw->getDIList()[dw->getUrlList().lastIndexOf(url)]->download == 2)
    {
        ui->downloadButton->setEnabled(true);
        ui->downloadButton->setText(tr("Download"));
    }
    if (dw->getDIList()[dw->getUrlList().lastIndexOf(url)]->download == 1)
    {
        ui->downloadButton->setEnabled(true);
        ui->downloadButton->setText(tr("Install"));
    }
    if (dw->getDIList()[dw->getUrlList().lastIndexOf(url)]->isInstall)
    {
        ui->downloadButton->setEnabled(false);
        ui->downloadButton->setText(tr("Installing"));
        return;
    }
    if (dw->getDIList()[dw->getUrlList().lastIndexOf(url)]->download == 3)
    {
        ui->downloadButton->setEnabled(true);
        ui->downloadButton->setText(tr("Reinstall"));
        ui->downloadButton->show();
        ui->pushButton_3->show();
    }
}

void AppIntoPage::sltAppinfoTags(QStringList *tagList)
{
    foreach (const QString &tag, *tagList)
    {
        if (tag == "community")
        {
            ui->tag_community->show();
        }
        else if (tag == "ubuntu")
        {
            ui->tag_ubuntu->show();
        }
        else if (tag == "deepin")
        {
            ui->tag_deepin->show();
        }
        else if (tag == "uos")
        {
            ui->tag_uos->show();
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
    }else {
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
AppIntoPage::~AppIntoPage()
{
    delete ui;
}

void AppIntoPage::on_downloadButton_clicked()
{
    if (ui->downloadButton->text() == tr("Install"))
    {
        dw->getDIList()[dw->getUrlList().lastIndexOf(SparkAPI::getServerUrl() + "store" + spk.path() + "/" + info["Filename"].toString())]->install(0);
        isDownloading(SparkAPI::getServerUrl() + "store" + spk.path() + "/" + info["Filename"].toString());
        QObject::connect(dw->getDIList()[dw->getUrlList().lastIndexOf(SparkAPI::getServerUrl() + "store" + spk.path() + "/" + info["Filename"].toString())], &DownloadItem::finished, [=]()
                         {
            isDownloading(SparkAPI::getServerUrl()+"store"+spk.path()+"/"+info["Filename"].toString());
            disconnect(dw->getDIList()[dw->getUrlList().lastIndexOf(SparkAPI::getServerUrl()+"store"+spk.path()+"/"+info["Filename"].toString())],&DownloadItem::finished,nullptr,nullptr); });
        return;
    }
    emit clickedDownloadBtn();
    dw->addItem(info["Name"].toString(), info["Filename"].toString(), info["Pkgname"].toString(), iconpixmap, SparkAPI::getServerUrl() + "store" + spk.path() + "/" + info["Filename"].toString());
    if (ui->downloadButton->text() == tr("Reinstall"))
    {
        dw->getDIList()[dw->allDownload - 1]->reinstall = true;
    }
    ui->downloadButton->setEnabled(false);
    QObject::connect(dw->getDIList()[dw->getUrlList().lastIndexOf(SparkAPI::getServerUrl() + "store" + spk.path() + "/" + info["Filename"].toString())], &DownloadItem::finished, [=]()
                     {
        isDownloading(SparkAPI::getServerUrl()+"store"+spk.path()+"/"+info["Filename"].toString());
        disconnect(dw->getDIList()[dw->getUrlList().lastIndexOf(SparkAPI::getServerUrl()+"store"+spk.path()+"/"+info["Filename"].toString())],&DownloadItem::finished,nullptr,nullptr); });
    isDownloading(SparkAPI::getServerUrl() + "store" + spk.path() + "/" + info["Filename"].toString());
}

void AppIntoPage::on_pushButton_3_clicked()
{
    QtConcurrent::run([=]()
    {
        ui->downloadButton->setEnabled(false);
        ui->pushButton_3->setEnabled(false);

        QProcess uninstall;
        uninstall.start("pkexec", QStringList() << "apt" << "purge" << "-y" << info["Pkgname"].toString().toLower());
        uninstall.waitForFinished(-1);

        QProcess check;
        check.start("dpkg", QStringList() << "-s" << info["Pkgname"].toString().toLower());
        check.waitForFinished(10*1000);

        if (check.readAllStandardOutput().isEmpty())
        {
            ui->downloadButton->setText(tr("Download"));
            ui->pushButton_3->hide();

            updatesEnabled();
            Utils::sendNotification("spark-store",tr("Spark Store"),tr("Uninstall succeeded"));
        }

        ui->downloadButton->setEnabled(true);
        ui->pushButton_3->setEnabled(true);
    });
}

void AppIntoPage::on_shareButton_clicked()
{
    qDebug() << "Share" << spk;
    Utils::sendNotification("spark-store", tr("Spark Store"), tr("The URL has been copied to the clipboard"));
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(spk.toString());
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
