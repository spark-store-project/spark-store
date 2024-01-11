#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QTextCodec>
class HttpRequest : public QObject
{
    Q_OBJECT
private:
    char*  rawHeaders;
public:
    HttpRequest(QObject *parent = nullptr);
    ~HttpRequest();

    void getRequest(QNetworkRequest request);

    QString postRequest(QString url,  QString jsondata);
signals:
    void finished(QString);
private slots:
    void readdata_slot(QNetworkReply *reply);
};

#endif // HTTPREQUEST_H

