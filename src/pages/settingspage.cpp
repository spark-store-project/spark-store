#include "settingspage.h"
#include "ui_settingspage.h"
bool SettingsPage::isdownload = false;
SettingsPage::SettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPage)
{
    ui->setupUi(this);
    initConfig();
}

void SettingsPage::setTheme(bool dark)
{
    if(dark)
    {
        this->setStyleSheet("#frame{background-color: #252525;border-radius:14px;border:1px solid rgb(64, 64, 64);}");
    }else {
        //亮色模式
        this->setStyleSheet("#frame{background-color: #ffffff;border-radius:14px;border:1px solid rgb(229,229,229);}");
    }
}

void SettingsPage::readServerList()
{
    // 读取服务器列表并初始化
    QFile file(QDir::homePath().toUtf8() + "/.config/spark-store/server.list");

    //判断文件是否存在
    if (!file.exists())
    {
        ui->comboBox_server->addItem("https://d.store.deepinos.org.cn/");
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"无法读取server.list";
    }

    //创建QTextStream对象
    QTextStream textStream(&file);

    QString lineData = textStream.readLine();//读取文件的第一行
    ui->comboBox_server->addItem(lineData);
    while(!lineData.isNull())
    {
        lineData = textStream.readLine();
        ui->comboBox_server->addItem(lineData);
    }
    for(int i = 0; i < ui->comboBox_server->count(); i++)
    {
        if(ui->comboBox_server->itemText(i) == "开发者模式 Dev only")
        {
            ui->comboBox_server->model()->setData(ui->comboBox_server->model()->index(i, 0), QVariant(0), Qt::UserRole - 1);
        }
        if(ui->comboBox_server->itemText(i) == "镜像源 Download only")
        {
            for(int j = i; j < ui->comboBox_server->count(); j++)
            {
                ui->comboBox_server->model()->setData(ui->comboBox_server->model()->index(j, 0), QVariant(0), Qt::UserRole - 1);
            }
            break;
        }
    }
    file.close();
}

void SettingsPage::initConfig()
{
    readServerList();

    // 读取服务器URL并初始化菜单项的链接
    QSettings readConfig(QDir::homePath() + "/.config/spark-store/config.ini", QSettings::IniFormat);
    if(!readConfig.value("server/choose").toString().isEmpty() && readConfig.value("server/updated").toString() == "TRUE")
    {
        ui->comboBox_server->setCurrentText(readConfig.value("server/choose").toString());
        SparkAPI::setServerUrl(readConfig.value("server/choose").toString());
    }
    configCanSave = true;   //　防止触发保存配置信号
}

SettingsPage::~SettingsPage()
{
    delete ui;
}

void SettingsPage::on_pushButton_updateServer_clicked()
{
    QtConcurrent::run([=]()
    {
        ui->pushButton_updateServer->setEnabled(false);
        ui->comboBox_server->clear();

        QFile::remove(QDir::homePath().toUtf8() + "/.config/spark-store/server.list");
        system("curl -o " + QDir::homePath().toUtf8() + "/.config/spark-store/server.list https://d.store.deepinos.org.cn/store/server-and-mirror.list");

        ui->pushButton_updateServer->setEnabled(true);
        ui->comboBox_server->setCurrentIndex(0);

        readServerList();
    });
}


void SettingsPage::on_comboBox_server_currentIndexChanged(const QString &arg1)
{
    SparkAPI::setServerUrl(arg1);  // 服务器信息更新

    const QString updatedInfo = "TRUE";
    if(configCanSave)
    {
        // ui->label_setting1->show();
        QSettings *setConfig = new QSettings(QDir::homePath() + "/.config/spark-store/config.ini", QSettings::IniFormat);
        setConfig->setValue("server/choose", arg1);
        setConfig->setValue("server/updated", updatedInfo);
        setConfig->deleteLater();
    }
}

void SettingsPage::setIsDownload(bool isdownload)
{
    SettingsPage::isdownload = isdownload;
}

void SettingsPage::updateUI()
{
    if(isdownload)
    {
        ui->pushButton_clear->setEnabled(false);
    }
    else
    {
        ui->pushButton_clear->setEnabled(true);
    }
    // 显示缓存占用空间
    quint64 tmp_size = dirFileSize(QString::fromUtf8(TMP_PATH));
    QString tmp_size_str;
    if(tmp_size < 1024)
    {
        tmp_size_str = QString::number(tmp_size) + "B";
    }
    else if(tmp_size < (1024 * 1024))
    {
        tmp_size_str = QString::number(0.01 * int(100 * (tmp_size / 1024))) + "KB";
    }
    else if(tmp_size<(1024*1024*1024))
    {
        tmp_size_str = QString::number(0.01 * int(100 * (tmp_size / (1024 * 1024)))) + "MB";
    }
    else
    {
        tmp_size_str = QString::number(0.01 * int(100 * (tmp_size / (1024 * 1024 * 1024)))) + "GB";
    }

    ui->tmp_size_ui->setText(tmp_size_str);
}

quint64 SettingsPage::dirFileSize(const QString &path)
{
    QDir dir(path);
    quint64 size = 0;
    // dir.entryInfoList(QDir::Files);  // 返回文件信息
    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Files))
    {
        // 计算文件大小
        size += quint64(fileInfo.size());
    }
    // dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);  // 返回所有子目录，并进行过滤
    foreach(QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        // 若存在子目录，则递归调用 dirFileSize() 函数
        size += dirFileSize(path + QDir::separator() + subDir);
    }
    return size;
}

void SettingsPage::on_pushButton_updateApt_clicked()
{
    QtConcurrent::run([=]()
    {
        ui->pushButton_updateApt->setEnabled(false);
        ui->label_aptserver->setText(tr("Updating, please wait..."));

        emit openUrl(QUrl("spk://store/tools/spark-store"));
        ui->label_aptserver->setText(tr(""));

        ui->pushButton_updateApt->setEnabled(true);
      });
}


void SettingsPage::on_pushButton_clear_clicked()
{
    QtConcurrent::run([=]()
    {
        ui->pushButton_clear->setEnabled(false);

        QDir tmpdir("/tmp/spark-store");
        tmpdir.setFilter(QDir::Files);
        int quantity = int(tmpdir.count());
        for(int i = 0; i < quantity; i++)
        {
            tmpdir.remove(tmpdir[i]);
        }
        ui->pushButton_clear->setEnabled(true);
        updateUI();
    });
}


void SettingsPage::on_pushButton_clearWebCache_clicked()
{
    QtConcurrent::run([=]()
    {
        QString dataLocal = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QDir cacheDir(dataLocal );
        cacheDir.removeRecursively();
    });
}

