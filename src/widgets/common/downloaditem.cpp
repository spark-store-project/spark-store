#include "downloaditem.h"
#include "ui_downloaditem.h"

#include <QtConcurrent>
#include <QProcess>

bool DownloadItem::isInstall = false;

DownloadItem::DownloadItem(QWidget *parent) :
    QWidget(parent),
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
}

DownloadItem::~DownloadItem()
{
    delete ui;
}

void DownloadItem::setValue(qint64 value)
{
    ui->progressBar->setValue(qint32(value));
    ui->label_2->setText(QString::number(double(value) / 100) + "% (" + speed + ")");
    if(ui->label_2->text().left(4) == "100%")
    {
        download = 1;
        ui->label_2->setText(tr("Download Complete."));
    }
}

void DownloadItem::setMax(qint64 max)
{
    ui->progressBar->setMaximum(qint32(max));
}

void DownloadItem::setName(QString name)
{
    ui->label->setText(name);
}

QString DownloadItem::getName()
{
    return ui->label_filename->text();
}

void DownloadItem::readyInstall()
{
    if(!close)
    {
        ui->progressBar->hide();
        ui->pushButton_install->setEnabled(true);
        ui->pushButton_install->show();
        DownloadItem::install(0);
        ui->pushButton_2->hide();
    }
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

void DownloadItem::install(int t)
{
    if(!isInstall)
    {
        isInstall = true;
        ui->pushButton_install->hide();
        ui->widget_spinner->show();
        qDebug() << "/tmp/spark-store/" + ui->label_filename->text().toUtf8();
        ui->label_2->setText(tr("Installing"));

        QtConcurrent::run([=]()
        {
            QProcess installer;
            switch(t)
            {
            case 0:
                installer.start("pkexec", QStringList() << "ssinstall" << "/tmp/spark-store/" + ui->label_filename->text().toUtf8());
                break;
            case 1:
                installer.start("deepin-deb-installer", QStringList() << "/tmp/spark-store/" + ui->label_filename->text().toUtf8());
                break;
            case 2:
                installer.start("pkexec", QStringList() << "gdebi" << "-n" << "/tmp/spark-store/" + ui->label_filename->text().toUtf8());
                break;
            }

            bool haveError = false;
            bool notRoot = false;
            installer.waitForFinished(-1); // 不设置超时
            out = installer.readAllStandardOutput();

            QStringList everyOut = out.split("\n");
            for(int i=0;i<everyOut.size();i++)
            {
                if(everyOut[i].left(2) == "E:")
                {
                    haveError = true;
                }
                if(everyOut[i].right(14) == "Not authorized")
                {
                    notRoot = true;
                }
            }

            QProcess isInstall;
            isInstall.start("dpkg -s " + pkgName);
            isInstall.waitForFinished(180*1000); // 默认超时 3 分钟
            int error = QString::fromStdString(isInstall.readAllStandardError().toStdString()).length();
            if(error == 0)
            {
                ui->pushButton_install->hide();
                Utils::sendNotification("spark-store",tr("Spark Store"),ui->label->text() + " " + tr("Installation complete."));
                ui->label_2->setText(tr("Finish"));
                download = 3;
                ui->pushButton_3->show();
            }
            else
            {
                ui->pushButton_install->show();
                ui->pushButton_install->setText(tr("Retry"));
                download = 1;
                Utils::sendNotification("spark-store",tr("Spark Store"),tr("Error happened in dpkg progress , you can try it again."));
                ui->label_2->setText(tr("Error happened in dpkg progress , you can try it again"));
                ui->pushButton_3->show();
            }

            if(notRoot)
            {
                Utils::sendNotification("spark-store",tr("Spark Store"),tr("dpkg progress had been aborted，you can retry installation."));
                ui->label_2->setText(tr("dpkg progress had been aborted，you can retry installation"));
                ui->pushButton_install->show();
                ui->pushButton_3->hide();
            }

            ui->widget_spinner->hide();
            DownloadItem::isInstall = false;
            emit finished();
        });
        //        emit finished();
        qDebug()<<ui->label_filename->text().toUtf8();
    }
}

void DownloadItem::on_pushButton_install_clicked()
{
    DownloadItem::install(0);
}

void DownloadItem::on_pushButton_2_clicked()
{
    ui->label_2->setText(tr("Download canceled"));
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
