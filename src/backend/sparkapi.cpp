#include "sparkapi.h"

#include <QDebug>

QString SparkAPI::serverUrl = "";

SparkAPI::SparkAPI(QObject *parent) : QObject(parent)
{
    QSettings readConfig(QDir::homePath() + "/.config/spark-store/config.ini", QSettings::IniFormat);
    if(!readConfig.value("server/choose").toString().isEmpty() && readConfig.value("server/updated").toString() == "TRUE")
    {
        SparkAPI::setServerUrl(readConfig.value("server/choose").toString());
    }
}
void SparkAPI::get(QUrl url)
{
    QNetworkRequest request;
    HttpRequest *httprequest=new HttpRequest;
    request.setUrl(QUrl(url.toString().replace("+","%2B")));
    connect(httprequest,&HttpRequest::finished,[=](QString data){
        QByteArray arr = data.toUtf8();
            //解析Json
            QJsonParseError error;
            if(QJsonDocument::fromJson(arr,&error).isArray())
            {
                auto doc = QJsonDocument::fromJson(arr,&error).array();
                emit finished(doc);
            }else {
                auto doc = QJsonDocument::fromJson(arr,&error).object();
                emit finishedObject(doc);
            }

            httprequest->deleteLater();
    });
    httprequest->getRequest(request);
}
void SparkAPI::getRAW(QUrl url)
{
    QNetworkRequest request;
    HttpRequest *httprequest=new HttpRequest;
    request.setUrl(QUrl(url.toString().replace("+","%2B")));
    connect(httprequest,&HttpRequest::finished,[=](QString data){
            emit finishedRAW(data);
            httprequest->deleteLater();
    });
    httprequest->getRequest(request);
}
void SparkAPI::getAppList(QString type)
{
    get(QUrl(getServerUrl()+"store/"+type+"/applist.json"));
}
void SparkAPI::getSearchList(QString keyword)
{
    get(QUrl("https://search.deepinos.org.cn/appinfo/search?keyword="+keyword));
}
void SparkAPI::getAppInfo(QUrl spk)
{
    get(QUrl(getServerUrl()+"store"+spk.path().replace("+","%2B") + "/app.json"));
}
void SparkAPI::getAppDownloadTimes(QUrl spk)
{
    getRAW(QUrl(getServerUrl()+"store"+spk.path().replace("+","%2B") + "/download-times.txt"));
}
QString SparkAPI::getServerUrl()
{
    return SparkAPI::serverUrl;
}
QString SparkAPI::getImgServerUrl()
{
    return SparkAPI::serverUrl;
}
void SparkAPI::setServerUrl(QString url)
{
    SparkAPI::serverUrl = url;
}
