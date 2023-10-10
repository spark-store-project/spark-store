#include "downloaditem.h"
#include "ui_downloaditem.h"

#include <QtConcurrent>
#include <QProcess>

bool DownloadItem::isInstall = false;

DownloadItem::DownloadItem(QWidget *parent) : QWidget(parent),
                                              reinstall(false),
                                              close(false),
                                              ui(new Ui::DownloadItem),
                                              output_w(new DDialog),
                                              textbrowser(new QTextBrowser)
{
    ui->setupUi(this);

    ui->pushButton_install->setEnabled(false);
    ui->progressBar->setValue(0);
    ui->label_filename->hide();
    ui->pushButton_install->hide();
    ui->pushButton_3->hide();
    ui->widget_spinner->start();
    ui->widget_spinner->hide();

    ui->label->setElideMode(Qt::TextElideMode::ElideRight);
    ui->label_2->setElideMode(Qt::TextElideMode::ElideRight);
}

DownloadItem::~DownloadItem()
{
    delete ui;
}

void DownloadItem::setValue(qint64 value)
{
    ui->progressBar->setValue(qint32(value));
    ui->label_2->setText(QString::number(double(value) / 100) + "% (" + speed + ")");
    ui->label_2->setToolTip(QString::number(double(value) / 100) + "% (" + speed + ")");
    if (ui->label_2->text().left(4) == "100%")
    {
        download = 1;
        ui->label_2->setText(tr("Download Complete."));
        ui->label_2->setToolTip(tr("Download Complete."));
    }
}

void DownloadItem::setMax(qint64 max)
{
    ui->progressBar->setMaximum(qint32(max));
}

void DownloadItem::setName(QString name)
{
    ui->label->setText(name);
    ui->label->setToolTip(name);
}

QString DownloadItem::getName()
{
    return ui->label_filename->text();
}


/***************************************************************
  *  @brief     告知界面，准备安装
  *  @param
  *  @note      如果正在安装，返回-1
  *  @Sample usage:     DownloadItem::install(0);
 **************************************************************/
int DownloadItem::readyInstall()
{
    // 检查是否正在安装，如果是返回错误 -1
    if (isInstall)
    {
        return -1;
    }

    if (!close)
    {
        ui->progressBar->hide();
        ui->pushButton_install->setEnabled(true);
        ui->pushButton_install->show();
        DownloadItem::install(0);
        ui->pushButton_2->hide();
        return 1;
    }
    return 0;
}

void DownloadItem::setFileName(QString fileName)
{
    ui->label_filename->setText(fileName);
}

void DownloadItem::seticon(const QPixmap icon)
{
    ui->label_3->setPixmap(icon);
}

void DownloadItem::closeDownload()
{
    on_pushButton_2_clicked();
}

void DownloadItem::setSpeed(QString s)
{
    speed = s;
}

/***************************************************************
  *  @brief     安装当前应用
  *  @param     int t, t为安装方式，可以为 0,1,2
  *  @note      执行这个函数时，需要已经检查是否可以安装，但该函数仍然会再检测一次！
  *  @Sample usage:     DownloadItem::install(0);
 **************************************************************/
void DownloadItem::install(int t)
{
    if (!isInstall)
    {
        isInstall = true;
        ui->pushButton_install->hide();
        ui->widget_spinner->show();
        qDebug() << "/tmp/spark-store/" + ui->label_filename->text().toUtf8();
        ui->label_2->setText(tr("Installing"));
        ui->label_2->setToolTip(tr("Installing"));

        QtConcurrent::run([=]()
        {
            slotAsyncInstall(t);
        });

        qDebug() << ui->label_filename->text().toUtf8();
    }
}

void DownloadItem::on_pushButton_install_clicked()
{
    DownloadItem::install(0);
}

void DownloadItem::on_pushButton_2_clicked()
{
    ui->label_2->setText(tr("Download canceled"));
    ui->label_2->setToolTip(tr("Download canceled"));
    download = 2;
    ui->pushButton_2->setEnabled(false);
    ui->progressBar->hide();
    close = true;
}

void DownloadItem::on_pushButton_3_clicked()
{
    textbrowser->setLineWidth(0);
    textbrowser->setText(out);
    textbrowser->setMinimumHeight(500);

    output_w->setMinimumHeight(600);
    output_w->setAttribute(Qt::WA_TranslucentBackground);
    output_w->setTitle(ui->label->text());
    output_w->layout()->setMargin(20);
    output_w->layout()->addWidget(textbrowser);
    output_w->show();
}

/***************************************************************
  *  @brief     实际安装应用
  *  @param     int t, t为安装方式，可以为 0,1,2
  *  @note      备注
  *  @Sample usage:     slotAsyncInstall(0);
 **************************************************************/
void DownloadItem::slotAsyncInstall(int t)
{
    QProcess installer;
    switch (t)
    {
    case 0:
        installer.start("pkexec", QStringList() << "ssinstall"
                                                << "/tmp/spark-store/" + ui->label_filename->text().toUtf8() << "--delete-after-install");
        break;
    case 1:
        installer.start("deepin-deb-installer", QStringList() << "/tmp/spark-store/" + ui->label_filename->text().toUtf8());
        break;
    case 2:
        installer.start("pkexec", QStringList() << "gdebi"
                                                << "-n"
                                                << "/tmp/spark-store/" + ui->label_filename->text().toUtf8());
        break;
    }

    bool haveError = false;
    bool notRoot = false;
    installer.waitForFinished(-1); // 不设置超时
    out = installer.readAllStandardOutput();

    QStringList everyOut = out.split("\n");
    QString tempOutput;
    for (int i = 0; i < everyOut.size(); i++)
    {
        tempOutput = everyOut[i];
        if (everyOut[i].left(2) == tempOutput.contains("OMG-IT-GOES-WRONG"))
        {
            haveError = true;
        }
        if (tempOutput.contains("Not authorized"))
        {
            notRoot = true;
        }
    }

    QProcess isInstall;
    isInstall.start("dpkg", QStringList() << "-s" << pkgName);
    isInstall.waitForFinished(180 * 1000); // 默认超时 3 分钟
    int error = QString::fromStdString(isInstall.readAllStandardError().toStdString()).length();
    if (error == 0 && !haveError)
    {
        ui->pushButton_install->hide();
        Utils::sendNotification("spark-store", tr("Spark Store"), ui->label->text() + " " + tr("Installation complete."));
        ui->label_2->setText(tr("Finish"));
        ui->label_2->setToolTip(tr("Finish"));
        download = 3;
        ui->pushButton_3->show();
    }
    else
    {
        ui->pushButton_install->show();
        ui->pushButton_install->setText(tr("Retry"));
        download = 1;
        Utils::sendNotification("spark-store", tr("Spark Store"), tr("Error happened in dpkg progress , please check the install info or try to reinstall."));
        ui->label_2->setText(tr("Error happened in dpkg progress , please check the install info or try to reinstall."));
        ui->label_2->setToolTip(tr("Error happened in dpkg progress , please check the install info or try to reinstall."));
        ui->pushButton_3->show();
    }

    if (notRoot)
    {
        Utils::sendNotification("spark-store", tr("Spark Store"), tr("dpkg progress had been aborted, please check the install info or try to reinstall."));
        ui->label_2->setText(tr("dpkg progress had been aborted, please check the install info or try to reinstall."));
        ui->label_2->setToolTip(tr("dpkg progress had been aborted, please check the install info or try to reinstall."));
        ui->pushButton_install->show();
        ui->pushButton_3->hide();
    }

    ui->widget_spinner->hide();
    DownloadItem::isInstall = false;

    installer.deleteLater();
    isInstall.deleteLater();

    emit finished(error == 0 && !haveError && !notRoot);
}
