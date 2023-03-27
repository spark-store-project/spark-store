#include "settingspage.h"
#include "ui_settingspage.h"
#include "../backend/sparkapi.h"
#include "utils/utils.h"

#include <QSettings>
#include <QtConcurrent>
#include <QDebug>

#define TMP_PATH "/tmp/spark-store"
#define DEFAULT_SERVER_URL "https://cdn.d.store.deepinos.org.cn/"
#define DEFAULT_CHECK_DOMAIN "deepinos"

bool SettingsPage::isdownload = false;

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsPage)
{
    ui->setupUi(this);

    configCanSave = false;
    initConfig();
}

void SettingsPage::setTheme(bool dark)
{
    if (dark)
    {
        this->setStyleSheet("#frame{background-color: #252525;border-radius:14px;border:1px solid rgb(64, 64, 64);}");
    }
    else
    {
        // 亮色模式
        this->setStyleSheet("#frame{background-color: #ffffff;border-radius:14px;border:1px solid rgb(229,229,229);}");
    }
}

void SettingsPage::readServerList()
{
    // 读取服务器列表并初始化
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/server.list");

    // 判断文件是否存在
    if (!file.exists())
    {
        ui->comboBox_server->addItem(DEFAULT_SERVER_URL);
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "无法读取 server.list";
        return;
    }

    // 创建 QTextStream 对象
    QTextStream textStream(&file);
    if (!textStream.readAll().contains(DEFAULT_CHECK_DOMAIN)) // 校验配置文件有效性
    {
        return;
    }
    textStream.seek(0);                       // 回到开头
    QString lineData = textStream.readLine(); // 读取文件的第一行
    ui->comboBox_server->addItem(lineData);
    while (!lineData.isNull())
    {
        lineData = textStream.readLine();
        ui->comboBox_server->addItem(lineData);
    }
    for (int i = 0; i < ui->comboBox_server->count(); i++)
    {
        if (ui->comboBox_server->itemText(i) == "开发者模式 Dev only")
        {
            ui->comboBox_server->model()->setData(ui->comboBox_server->model()->index(i, 0), QVariant(0), Qt::UserRole - 1);
        }
        if (ui->comboBox_server->itemText(i) == "镜像源 Download only")
        {
            for (int j = i; j < ui->comboBox_server->count(); j++)
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
    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    if (!config.value("server/choose").toString().isEmpty() && config.value("server/updated").toBool())
    {
        qDebug() << config.value("server/choose").toString();
        ui->comboBox_server->setCurrentText(config.value("server/choose").toString());
        SparkAPI::setServerUrl(config.value("server/choose").toString());
    }
    configCanSave = true; // 　防止触发保存配置信号
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


        QFile::remove(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/server.list");
        auto updateSuccess = system("curl -o " + QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toUtf8() + "/server.list " /* 注意空格的问题 */ + DEFAULT_SERVER_URL + "store/server-and-mirror.list");
        qDebug() << "Update serverlist status:" << updateSuccess;
        if (updateSuccess != 0) // 更新失败不换服务器配置
        {
            QFile file(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/server.list");
            if (file.exists())
            {
                file.remove();
            }
            // FIXME: 向用户提示更新失败
            ui->pushButton_updateServer->setEnabled(true);
            return;
        }
        ui->comboBox_server->clear();
        ui->pushButton_updateServer->setEnabled(true);
        readServerList();
        ui->comboBox_server->setCurrentIndex(0); });
}

void SettingsPage::on_comboBox_server_currentIndexChanged(const QString &arg1)
{
    SparkAPI::setServerUrl(arg1); // 服务器信息更新
    qDebug() << arg1;
    bool updatedInfo = true;
    if(configCanSave)
    {
        // ui->label_setting1->show();
        QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
        config.setValue("server/choose", arg1);
        config.setValue("server/updated", updatedInfo);
        config.sync();
    }
}

void SettingsPage::setIsDownload(bool isdownload)
{
    SettingsPage::isdownload = isdownload;
}

void SettingsPage::updateUI()
{
    if (isdownload)
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
    if (tmp_size < 1024)
    {
        tmp_size_str = QString::number(tmp_size) + "B";
    }
    else if (tmp_size < (1024 * 1024))
    {
        tmp_size_str = QString::number(0.01 * int(100 * (tmp_size / 1024))) + "KB";
    }
    else if (tmp_size < (1024 * 1024 * 1024))
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
    foreach (QFileInfo fileInfo, dir.entryInfoList(QDir::Files))
    {
        // 计算文件大小
        size += quint64(fileInfo.size());
    }
    // dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);  // 返回所有子目录，并进行过滤
    foreach (QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
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

        QDir tmpdir(QString::fromUtf8(TMP_PATH));
        tmpdir.setFilter(QDir::Files);
        int quantity = int(tmpdir.count());
        for(int i = 0; i < quantity; i++)
        {
            tmpdir.remove(tmpdir[i]);
        }
        Utils::sendNotification("spark-store",tr("Spark Store"),tr("Temporary cache was cleaned"));
        ui->pushButton_clear->setEnabled(true);
        updateUI(); });
}

void SettingsPage::on_pushButton_clearWebCache_clicked()
{
    QtConcurrent::run([=]()
    {
        QString localDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/QtWebEngine";
        qDebug() << localDataLocation;
        QDir dataDir(localDataLocation);
        dataDir.removeRecursively();

        QString cacheLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/QtWebEngine";
        qDebug() << cacheLocation;
        QDir cacheDir(cacheLocation);
        cacheDir.removeRecursively();
    });
}
