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

    // Read /etc/os-release file
    QFile osReleaseFile("/etc/os-release");
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
        qWarning() << "Could not open /etc/os-release file";
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
    QUrl url("https://status.deepinos.org.cn/upload");

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
