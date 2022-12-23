#include "httprequest.h"

HttpRequest::HttpRequest()
{
}

void HttpRequest::getRequest(QNetworkRequest request)
{
    QNetworkAccessManager *naManager = new QNetworkAccessManager(this);

    request.setRawHeader("User-Agent", "Mozilla/5.0");
    request.setRawHeader("Content-Type", "charset='utf-8'");
    request.setRawHeader("Content-Type", "application/json");

    naManager->get(request);
    QObject::connect(naManager, &QNetworkAccessManager::finished, this, &HttpRequest::readdata_slot);
}
void HttpRequest::readdata_slot(QNetworkReply *reply)
{
    emit finished(reply->readAll());
}
QString HttpRequest::postRequest(QString url, QString jsondata)
{
    QByteArray array = jsondata.toLatin1();
    QNetworkRequest request;
    QNetworkAccessManager *naManager = new QNetworkAccessManager(this);
    QUrl strUrl = url.replace("+", "%2B");
    request.setUrl(strUrl);
    request.setRawHeader("Content-Type", "charset='utf-8'");
    request.setRawHeader("Content-Type", "application/json");

    QNetworkReply *reply = naManager->post(request, array);
    QEventLoop eventLoop;
    connect(naManager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QString strReply = codec->toUnicode(reply->readAll());
    reply->deleteLater();
    return strReply;
}
