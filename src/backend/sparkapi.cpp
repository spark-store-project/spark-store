#include "sparkapi.h"

#include <QDebug>
SparkAPI::SparkAPI(QObject *parent) : QObject(parent)
{

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
    return "https://cdn.d.store.deepinos.org.cn/";
}
QString SparkAPI::getImgServerUrl()
{
    return "https://cdn.d.store.deepinos.org.cn/";
}
