#ifndef SPARKAPI_H
#define SPARKAPI_H
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include "utils/httprequest.h"

class SparkAPI : public QObject
{
    Q_OBJECT
public:
    static QString getServerUrl();
    static QString getImgServerUrl();
    static void setServerUrl(QString url);
    void getSearchList(QString keyword);
    void get(QUrl url);
    void getAppList(QString type);
    void getAppInfo(QUrl spk);
    explicit SparkAPI(QObject *parent = nullptr);

private:
    static QString serverUrl;

signals:
    void finished(QJsonArray);
    void finishedObject(QJsonObject);

public slots:
};

#endif // SPARKAPI_H
