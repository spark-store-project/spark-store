#include "sparkapi.h"

#include <QDebug>

QString SparkAPI::serverUrl = "";

SparkAPI::SparkAPI(QObject *parent) : QObject(parent)
{
   SparkAPI::serverUrl = "https://cdn.d.store.deepinos.org.cn/";
}
void SparkAPI::get(QUrl url)
{
    QNetworkRequest request;
    HttpRequest *httprequest=new HttpRequest;
    request.setUrl(url);
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
    get(QUrl(getServerUrl()+"store"+spk.path() + "/app.json"));
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
