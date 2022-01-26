
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include "spkutils.h"

void SpkUtils::VerifySingleRequest(QPointer<QNetworkReply> aReply)
{
  if(aReply.isNull())
    return;
  aReply->disconnect(SIGNAL(finished()));
  aReply->abort();
  aReply->deleteLater();
}

QString SpkUtils::GetDistroName()
{
  QSettings osRelease("/etc/os-release", QSettings::IniFormat);
  return osRelease.value("PRETTY_NAME", "Unknown Distro").toString() + "-" +
         osRelease.value("BUILD_ID", "Unknown Build").toString();
}

int SpkUtils::VerifyReplyJson(QNetworkReply *aReply, QJsonValue &aRetDoc)
{
  QJsonParseError err;
  QByteArray rawjson = aReply->readAll();
//  qDebug() << "Received:" << rawjson;
  QJsonDocument ret = QJsonDocument::fromJson(rawjson, &err);
  QJsonObject replyObject;
  if(err.error != QJsonParseError::NoError)
  {
    sErr(QObject::tr("VerifyReplyJson: returned JSON of request to %1 is unreadable.")
         .arg(aReply->url().toString()));
    return err.error;
  }
  if(!ret.isObject())
  {
    sErr(QObject::tr("VerifyReplyJson: returned JSON of request to %1 is not an Object.")
         .arg(aReply->url().toString()));
    return -1;
  }
  replyObject = ret.object();
  if(!replyObject.contains("code"))
  {
    sWarn(QObject::tr("VerifyReplyJson: reply of request to %1 doesn't have a code.")
          .arg(aReply->url().toString()));
  }
  else
  {
    auto OpRetCode = replyObject.value("code");
    if(!OpRetCode.isDouble())
    {
      sWarn(QObject::tr("VerifyReplyJson: Reply of request to %1 has a non-numeric code.")
            .arg(aReply->url().toString()));
    }
    else if(OpRetCode.toInt() != 0)
    {
      sNotify(QObject::tr("VerifyReplyJson: Server sent back an failure message; code: %1.")
              .arg(OpRetCode.toInt()));
      sErr(QObject::tr("VerifyReplyJson: Request to %1 failed with code %2.")
           .arg(aReply->url().toString()).arg(OpRetCode.toInt()));
      return OpRetCode.toInt();
    }
  }
  if(!replyObject.contains("data"))
  {
    sNotify(QObject::tr("VerifyReplyJson: Server did not reply with any data."));
    sErr(QObject::tr("VerifyReplyJson: Reply of request to %1 didn't include any data.")
         .arg(aReply->url().toString()));
    return -2;
  }
  aRetDoc = replyObject.value("data");
  return 0;
}

void SpkUtils::DeleteReplyLater(QNetworkReply *aReply)
{
  QObject::connect(aReply, &QNetworkReply::finished, aReply, &QObject::deleteLater);
}

QString SpkUtils::CutFileName(QString path)
{
  return path.section('/', -1);
}

QString SpkUtils::CutPath(QString path)
{
  return path.section('/', 1, -2, QString::SectionIncludeLeadingSep);
}

QString SpkUtils::BytesToSize(size_t s, int prec)
{
  if(s > (1 << 30))
    return QString::number(double (s) / (1 << 30), 'f', prec) + " GB";
  if(s > (1 << 20))
    return QString::number(double (s) / (1 << 20), 'f', prec) + " MB";
  if(s > (1 << 10))
    return QString::number(double (s) / (1 << 10), 'f', prec) + " KB";
  return QString::number(s) + " B";
}

bool SpkUtils::EnsureDirExists(QString path)
{
  QDir dir;

  if(!dir.exists(path))
    if(!dir.mkpath(path))
      return false;

  return true;
}
