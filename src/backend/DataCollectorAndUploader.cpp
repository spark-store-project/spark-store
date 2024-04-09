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
    QString version = config.value("build/version").toString();
    QString uuid = config.value("info/uuid").toString();


    // Execute lsb_release --all and capture the output
    QProcess lsbProcess;
    lsbProcess.start("lsb_release", QStringList() << "--all");
    lsbProcess.waitForFinished();
    QString lsbOutput = lsbProcess.readAllStandardOutput();

    QStringList lines = lsbOutput.split('\n');
    for (const QString &line : lines) {
        if (line.contains("Distributor ID:")) {
            distributor_id = line.split(":").last().trimmed();
        } else if (line.contains("Release:")) {
            release = line.split(":").last().trimmed();
        }
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



//    // Convert to byte array
//    QJsonDocument doc(json);
//    QByteArray jsonData = doc.toJson();

//    // Initialize a network request
//    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
//    QUrl url("https://status.deepinos.org.cn/upload");

//    QNetworkRequest request(url);
//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

//    // Send the POST request
//    QNetworkReply *reply = manager->post(request, jsonData);

//    connect(reply, &QNetworkReply::finished, [=]() {
//        if (reply->error() == QNetworkReply::NoError) {
//            emit uploadSuccessful();
//        } else {
//            emit uploadFailed(reply->errorString());
//        }
//        reply->deleteLater();
//    });
}
