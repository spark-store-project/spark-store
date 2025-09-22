#include "settingspage.h"
#include "ui_settingspage.h"
#include "../backend/sparkapi.h"
#include "utils/utils.h"

#include <QSettings>
#include <QtConcurrent>
#include <QDebug>
#include <QMessageBox>

#define TMP_PATH "/tmp/spark-store"
#define DEFAULT_SERVER_URL "https://cdn-d.spark-app.store/"


bool SettingsPage::needUncompatibleNotification = true;
bool SettingsPage::isdownload = false;

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsPage)
{
    ui->setupUi(this);

    configCanSave = false;
    initConfig();
    
    // 连接导出日志按钮的点击信号
    connect(ui->pushButton_exportLog, &QPushButton::clicked, this, &SettingsPage::on_pushButton_exportLog_clicked);
}

void SettingsPage::setTheme(bool dark)
{
    if (dark)
    {
        this->setStyleSheet("#frame{background-color: #252525;border-radius:14px;border:1px solid rgb(64, 64, 64);}");
    }
    else
    {
        // ���色模式
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

    // 在现有代码后添加初始化checkBox_disableSandbox的状态
    needUncompatibleNotification = config.value("other/uncompatibleNotification", needUncompatibleNotification).toBool();
    ui->checkBox->setChecked(needUncompatibleNotification);

    // 新增：从config.ini读取webengine/noSandbox配置并设置复选框状态
    bool disableSandbox = config.value("webengine/noSandbox", false).toBool();
    ui->checkBox_disableSandbox->setChecked(disableSandbox);

}

SettingsPage::~SettingsPage()
{
    delete ui;
}

void SettingsPage::on_pushButton_updateServer_clicked()
{
    auto future = QtConcurrent::run([=]()
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

void SettingsPage::on_comboBox_server_currentTextChanged(const QString &arg1)
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
    auto future = QtConcurrent::run([=]()
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
    auto future = QtConcurrent::run([=]()
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
    auto future = QtConcurrent::run([=]()
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

void SettingsPage::on_checkBox_clicked(bool checked)
{
    needUncompatibleNotification = checked;
    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    config.setValue("other/uncompatibleNotification", needUncompatibleNotification);
    config.sync();
}

// 添加checkBox_disableSandbox的点击事件处理函数
void SettingsPage::on_checkBox_disableSandbox_clicked(bool checked)
{
    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    config.setValue("webengine/noSandbox", checked);
    config.sync();
}

// 添加导出日志按钮的点击事件处理函数
void SettingsPage::on_pushButton_exportLog_clicked()
{
    auto future = QtConcurrent::run([=]() {
        // 禁用按钮防止重复点击
        QMetaObject::invokeMethod(ui->pushButton_exportLog, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, false));
        
        QString targetPath = QString::fromUtf8(TMP_PATH);
        bool success = Utils::exportLogs(targetPath);
        
        // 显示导出结果通知
        QString message;
        if (success) {
            message = tr("Logs exported successfully to: %1").arg(targetPath);
            Utils::writeLog("INFO", "User exported logs via settings page");
        } else {
            message = tr("Failed to export logs");
            Utils::writeLog("ERROR", "User failed to export logs via settings page");
        }
        
        // 在主线程显示消息框
        QMetaObject::invokeMethod(this, [message, this]() {
            QMessageBox::information(this, tr("Export Logs"), message);
            // 重新启用按钮
            ui->pushButton_exportLog->setEnabled(true);
        }, Qt::QueuedConnection);
    });
}