
#pragma once

#include <QJsonDocument>
#include <QString>
#include <QSettings>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

#include "spklogging.h"
#include "spkmainwindow.h"

/**
 * @brief SpkStore class is the core of the store client side program, it is constructed first and
 *        handling all processing after the launch. All client side data should be held by it,
 *        including preferences class, category names, request URLs, downloads status, packaging
 *        backend interfaces and so on.
 */
class SpkStore : public QObject
{
    Q_OBJECT
  public:
    static SpkStore *Instance;
    QSettings *mCfg;
    SpkStore(bool aCli, QString &aLogPath);
    ~SpkStore();

    SpkMainWindow* GetRootWindow() { return mMainWindow; }

    void SetApiResuestUrl(QString aUrlStr) { mApiRequestUrl = aUrlStr; }
    QString GetApiRequestUrl() { return mApiRequestUrl; }
    QNetworkReply *SendApiRequest(QString path, QJsonDocument param = QJsonDocument());

  private:
    SpkLogger *mLogger;
    SpkMainWindow *mMainWindow = nullptr;
    QNetworkAccessManager *mNetMgr = nullptr;
    QString mDistroName, mApiRequestUrl, mUserAgentStr, mConfigPath;
};
