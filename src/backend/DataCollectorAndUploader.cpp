#include "DataCollectorAndUploader.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

DataCollectorAndUploader::DataCollectorAndUploader(QObject *parent) : QObject(parent)
{
}

void DataCollectorAndUploader::collectAndUploadData()
{
    collectData();
}

void DataCollectorAndUploader::collectData()
{
    QString distributor_id;
    QString release;
    QString architecture;

    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    QString version = QString(APP_VERSION); // 使用编译时定义的版本号
    QString uuid = config.value("info/uuid").toString();

    // 根据环境变量选择 os-release 文件路径
    QString osReleasePath = "/etc/os-release";
    if (qEnvironmentVariableIsSet("IS_ACE_ENV") && qgetenv("IS_ACE_ENV") == "1") {
        osReleasePath = "/host/etc/os-release";
    }

    // Read os-release file
    QFile osReleaseFile(osReleasePath);
    if (osReleaseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&osReleaseFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("ID=")) {
                distributor_id = line.mid(3).remove('"').trimmed();
            } else if (line.startsWith("VERSION_ID=")) {
                release = line.mid(11).remove('"').trimmed();
            }
        }
        osReleaseFile.close();
    } else {
        qWarning() << "Could not open os-release file:" << osReleasePath;
    }

    // Execute uname -m to get the architecture
    QProcess unameProcess;
    unameProcess.start("uname", QStringList() << "-m");
    unameProcess.waitForFinished();
    architecture = unameProcess.readAllStandardOutput().trimmed();

    // Create a JSON object
    QJsonObject json;
    json.insert("Distributor ID", distributor_id);
    json.insert("Release", release);
    json.insert("Architecture", architecture);
    json.insert("Store_Version", version);
    json.insert("UUID", uuid);

    // Convert to byte array
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson();

    // Initialize a network request
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QUrl url("https://status.spark-app.store/upload");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Send the POST request
    QNetworkReply *reply = manager->post(request, jsonData);

    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit uploadSuccessful();
        } else {
            emit uploadFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}