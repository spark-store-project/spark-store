#include "downloadlistwidget.h"
#include "ui_downloadlistwidget.h"
#include "widgets/common/downloaditem.h"
#include "backend/sparkapi.h"
#include "backend/downloadworker.h"
#include "utils/utils.h"
#include "application.h"
#include "mainwindow-dtk.h"

#include <QDesktopServices>
#include <QtConcurrent>
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
        // 这里没有释放 downloadController，使用懒汉式单例
    }

    clearItem();
    delete ui;
}

bool DownloadListWidget::isDownloadInProcess()
{
    if (toDownload > 0)
    {
        return true;
    }
    return false;
}

void DownloadListWidget::clearItem()
{
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
    toDownload += 1;

    DownloadItem *di = new DownloadItem;
    connect(di, &DownloadItem::finished, this, &DownloadListWidget::slotInstallFinished, Qt::QueuedConnection);

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

    // 使用懒汉式单例来存储downloadController
    if (downloadController == nullptr)
    {
        downloadController = new DownloadController; // 并发下载，在第一次点击下载按钮的时候才会初始化
    }
    else
    {
        downloadController->disconnect();
        downloadController->stopDownload();
    }

    connect(downloadController, &DownloadController::downloadProcess, this, &DownloadListWidget::updateDataReadProgress);
    connect(downloadController, &DownloadController::downloadFinished, this, &DownloadListWidget::httpFinished);
    // connect(downloadController, &DownloadController::errorOccur, this, [=](QString msg){this->sendNotification(msg);});
    downloadController->setFilename(fileName);
    downloadController->startDownload(url.toString());
}

/***************************************************************
  *  @brief     下载列表完成下载的回调函数
  *  @param     
  *  @note      如果正在安装，则在新开的线程空间中等待上一个安装完
  *  @Sample usage:     
 **************************************************************/
void DownloadListWidget::httpFinished() // 完成下载
{
    isdownload = false;
    isBusy = false;

    QtConcurrent::run([=]()
    {
        while (downloaditemlist[nowDownload - 1]->readyInstall() == -1) // 安装当前应用，堵塞安装，后面的下载suspend
        {
            QThread::msleep(500);  // 休眠500ms，减少CPU负担
            continue;
        }
        downloaditemlist[nowDownload - 1]->free = true;
        emit downloadFinished();

        if (nowDownload < allDownload)
        {
            // 如果有排队则下载下一个
            qDebug() << "Download: 切换下一个下载...";
            nowDownload += 1;
            while (nowDownload <= allDownload && downloaditemlist[nowDownload - 1]->close)
            {
                nowDownload += 1;
            }
            if (nowDownload <= allDownload)
            {
                QString fileName = downloaditemlist[nowDownload - 1]->getName();
                startRequest(urList.at(nowDownload - 1), fileName);
            }
        }
    });
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

void DownloadListWidget::mouseMoveEvent(QMouseEvent *event)
{
    setGeometry(m_rect);
}

void DownloadListWidget::on_pushButton_clicked()
{
    QDesktopServices::openUrl(QUrl("file:///tmp/spark-store", QUrl::TolerantMode));
}

void DownloadListWidget::slotInstallFinished(bool success)
{
    // NOTE: 仅在安装成功后判断是否需要退出后台
    if (!success) {
        qDebug() << "Download: install failed";
        return;
    }

    if (toDownload > 0) {
        toDownload -= 1;
        qDebug() << "Download: toDownload" << toDownload;
    }

    if (toDownload == 0) {
        Application *app = qobject_cast<Application *>(qApp);
        MainWindow *mainWindow = app->mainWindow();
        if (mainWindow->isCloseWindowAnimation() == true)
        {
            qDebug() << "Download: 后台安装结束，退出程序";
            qApp->quit();
        }
    }
}
