
#pragma once

#include <QPointer>
#include <QString>
#include <QSettings>
#include <QFile>
#include <QtNetwork/QNetworkReply>

#include "spkstore.h"
#include "spklogging.h"

#define STORE (SpkStore::Instance)
#define CFG (SpkStore::Instance->mCfg)
#define RES (SpkResource::Instance)
#define PKG (SpkPkgMgrBase::Instance())

namespace SpkUtils
{
  QString GetDistroName();

  void VerifySingleRequest(QPointer<QNetworkReply> aReply);

  void DeleteReplyLater(QNetworkReply *aReply);

  int VerifyReplyJson(QNetworkReply *aReply, QJsonValue& aRetDoc);

  QString CutFileName(QString);
  QString CutPath(QString);
  QString BytesToSize(size_t s, int prec = 2);
  bool EnsureDirExists(QString path);

  bool FindViableTerminal();

  extern QPair<QString, QString> AvailableTerminal;
}
