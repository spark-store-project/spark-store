#ifndef SPARKAPI_H
#define SPARKAPI_H
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include "utils/httprequest.h"
#include <QSettings>
#include <QDir>
class SparkAPI : public QObject
{
    Q_OBJECT
public:
    static QString getServerUrl();
    static QString getImgServerUrl();
    static void setServerUrl(QString url);
    void getAppDownloadTimes(QUrl spk);
    void getSearchList(QString keyword);
    void get(QUrl url);
    void getRAW(QUrl url);
    void getAppList(QString type);
    void getAppInfo(QUrl spk);
    explicit SparkAPI(QObject *parent = nullptr);

private:
    static QString serverUrl;

signals:
    void finished(QJsonArray);
    void finishedRAW(QString);
    void finishedObject(QJsonObject);

public slots:
};

#endif // SPARKAPI_H
