
#pragma once

#include <QJsonDocument>
#include <QString>
#include <QSettings>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

#include "spklogging.h"
#include "spkresource.h"

class SpkMainWindow;

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

    void SetApiRequestUrl(QString aUrlStr) { mApiRequestUrl = aUrlStr; }
    QString GetApiRequestUrl() { return mApiRequestUrl; }
    QNetworkReply *SendApiRequest(QString path, QJsonDocument param = QJsonDocument());
    QNetworkReply *SendResourceRequest(QString path); ///< WARNING: Only intended for SpkResource!
    QNetworkReply *SendDownloadRequest(QUrl file, qint64 fromByte = -1, qint64 toByte = -1);

    QNetworkReply *SendCustomHeadRequest(QNetworkRequest);

  private:
    SpkLogger *mLogger;
    SpkMainWindow *mMainWindow = nullptr;
    SpkResource *mResMgr = nullptr;
    QNetworkAccessManager *mNetMgr = nullptr;
    QString mDistroName,
            mApiRequestUrl,
            mResourceRequestUrl,
            mUserAgentStr,
            mConfigPath;
};
