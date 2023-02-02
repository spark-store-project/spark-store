#include "downloadlistwidget.h"
#include "ui_downloadlistwidget.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDebug>
DownloadListWidget::DownloadListWidget(QWidget *parent) : DBlurEffectWidget(parent),
                                                          ui(new Ui::DownloadListWidget)
{
    ui->setupUi(this);
    setWindowTitle(QObject::tr("Download list"));
    installEventFilter(this);
    this->setAttribute(Qt::WA_Hover, true);
    setFocus();
    setFixedSize(500, 400);
    setMaskAlpha(250);
    ui->listWidget->hide();
    ui->widget->show();
    // 计算显示下载速度
    download_speed.setInterval(1000);
    download_speed.start();
    connect(&download_speed, &QTimer::timeout, [=]()
            {
        if(isdownload && theSpeed == "")
        {
            size1 = download_size;
            double bspeed;
            bspeed = size1 - size2;
            if(bspeed < 1024)
            {
                theSpeed = QString::number(bspeed) + "B/s";
            }
            else if(bspeed < 1024 * 1024)
            {
                theSpeed = QString::number(0.01 * int(100 * (bspeed / 1024))) + "KB/s";
            }
            else if(bspeed < 1024 * 1024 * 1024)
            {
                theSpeed = QString::number(0.01 * int(100 * (bspeed / (1024 * 1024)))) + "MB/s";
            }
            else
            {
                theSpeed = QString::number(0.01 * int(100 * (bspeed / (1024 * 1024 * 1024)))) + "GB/s";
            }
            size2 = download_size;
        }
        if(isdownload){
            downloaditemlist[nowDownload - 1]->setSpeed(theSpeed);
        }else{
            emit downloadProgress(0);
        } });
}

DownloadListWidget::~DownloadListWidget()
{
    if (downloadController)
    {
        downloadController->disconnect();
        downloadController->stopDownload();
        downloadController->deleteLater();
    }

    clearItem();
    delete ui;
}

void DownloadListWidget::clearItem()
{
    //    QListWidgetItem *item = nullptr;
    //    while ((item = ui->listWidget->takeItem(0)) != nullptr)
    //    {
    //        QWidget *card = ui->listWidget->itemWidget(item);
    //        if (card)
    //        {
    //            card->deleteLater();
    //            card = nullptr;
    //        }
    //        delete item;
    //        item = nullptr;
    //    }

    //    ui->listWidget->vScrollBar->scrollTop();
    ui->listWidget->clear();
}
DownloadItem* DownloadListWidget::addItem(QString name, QString fileName, QString pkgName, const QPixmap icon, QString downloadurl)
{
    if (fileName.isEmpty())
    {
        return nullptr;
    }
    urList.append(downloadurl);
    allDownload += 1;
    DownloadItem *di = new DownloadItem;
    dlist << downloadurl;
    downloaditemlist << di;
    di->setName(name);
    di->setFileName(fileName);
    di->pkgName = pkgName;
    di->seticon(icon);
    QListWidgetItem *pItem = new QListWidgetItem();
    pItem->setSizeHint(QSize(240, 56)); // ui 中单个 downloaditem 高度固定 56px（上下 margin 8px，图片固定 48x48）
    ui->listWidget->addItem(pItem);
    ui->listWidget->setItemWidget(pItem, di);
    if (!isBusy)
    {
        nowDownload += 1;
        startRequest(urList.at(nowDownload - 1), fileName); // 进行链接请求
    }

    return di;
}

QList<DownloadItem *> DownloadListWidget::getDIList()
{
    return downloaditemlist;
}

QList<QUrl> DownloadListWidget::getUrlList()
{
    return urList;
}

void DownloadListWidget::startRequest(QUrl url, QString fileName)
{
    ui->listWidget->show();
    ui->widget->hide();
    isBusy = true;
    isdownload = true;
    downloaditemlist[allDownload - 1]->free = false;

    if (downloadController)
    {
        downloadController->disconnect();
        downloadController->stopDownload();
        downloadController->deleteLater();
    }
    downloadController = new DownloadController; // 并发下载，在点击下载按钮的时候才会初始化
    connect(downloadController, &DownloadController::downloadProcess, this, &DownloadListWidget::updateDataReadProgress);
    connect(downloadController, &DownloadController::downloadFinished, this, &DownloadListWidget::httpFinished);
    // connect(downloadController, &DownloadController::errorOccur, this, [=](QString msg){this->sendNotification(msg);});
    downloadController->setFilename(fileName);
    downloadController->startDownload(url.toString());
}

void DownloadListWidget::httpFinished() // 完成下载
{
    isdownload = false;
    isBusy = false;
    downloaditemlist[nowDownload - 1]->readyInstall();
    downloaditemlist[nowDownload - 1]->free = true;
    emit downloadFinished();
    if (nowDownload < allDownload)
    {
        // 如果有排队则下载下一个
        qDebug() << "切换下一个下载...";
        nowDownload += 1;
        while (downloaditemlist[nowDownload - 1]->close)
        {
            nowDownload += 1;
            if (nowDownload >= allDownload)
            {
                nowDownload = allDownload;
                return;
            }
        }
        QString fileName = downloaditemlist[nowDownload - 1]->getName();
        startRequest(urList.at(nowDownload - 1), fileName);
    }
}

void DownloadListWidget::updateDataReadProgress(QString speedInfo, qint64 bytesRead, qint64 totalBytes)
{
    if (totalBytes <= 0)
    {
        return;
    }
    theSpeed = speedInfo;
    downloaditemlist[nowDownload - 1]->setMax(10000);                                     // 最大值
    downloaditemlist[nowDownload - 1]->setValue(int(bytesRead * 100 / totalBytes) * 100); // 当前值
    emit downloadProgress(int(bytesRead * 100 / totalBytes));
    download_size = bytesRead;
    if (downloaditemlist[nowDownload - 1]->close)
    {
        // 随时检测下载是否被取消
        downloadController->disconnect();
        downloadController->stopDownload();
        downloaditemlist[nowDownload - 1]->closeDownload();
        httpFinished();
    }
}

void DownloadListWidget::m_move(int x, int y)
{
    m_rect.setX(x);
    m_rect.setY(y);
    move(x, y);
    return;
}
bool DownloadListWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (Q_NULLPTR == watched)
    {
        return false;
    }
    if (QEvent::ActivationChange == event->type())
    {
        if (QApplication::activeWindow() != this)
        {
            this->close();
        }
    }
    return QWidget::eventFilter(watched, event);
}
void DownloadListWidget::mouseMoveEvent(QMouseEvent *event)
{
    setGeometry(m_rect);
}

void DownloadListWidget::on_pushButton_clicked()
{
    QDesktopServices::openUrl(QUrl("file:///tmp/spark-store", QUrl::TolerantMode));
}
