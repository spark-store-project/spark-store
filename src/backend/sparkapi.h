#ifndef SPARKAPI_H
#define SPARKAPI_H

#include "utils/httprequest.h"

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDir>

class SparkAPI : public QObject
{
    Q_OBJECT

public:
    explicit SparkAPI(QObject *parent = nullptr);

    static QString getServerUrl();
    static QString getImgServerUrl();
    static void setServerUrl(QString url);
    void getAppDownloadTimes(QUrl spk);
    void getSearchList(QString keyword);
    void get(QUrl url);
    void getRAW(QUrl url);
    void getAppList(QString type);
    void getAppInfo(QUrl spk);
    static QString getArchDir();

signals:
    void finished(QJsonArray);
    void finishedRAW(QString);
    void finishedObject(QJsonObject);

private:
    static QString serverUrl;
    static QString serverUrlDir;
};

#endif // SPARKAPI_H
