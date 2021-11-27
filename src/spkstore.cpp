
#include <spkui_general.h>
#include <QPluginLoader>
#include <QDir>
#include <QApplication>
#include <QtNetwork/QNetworkProxy>

#include "dtk/spkdtkplugin.h"
#include "gitver.h"
#include "spkpopup.h"
#include "spkstore.h"
#include "spkutils.h"

SpkStore *SpkStore::Instance = nullptr;
static void InstallDefaultConfigs();

SpkStore::SpkStore(bool aCli, QString &aLogPath)
{
  mLogger = new SpkLogger(aLogPath);

  // Singleton
  Q_ASSERT(Instance == nullptr);
  Instance = this;

  // Finish all essential initialization after this.

  mConfigPath = QDir::homePath() + "/.config/spark-store/config"; //TODO: flexible config via CLI
  if(QFileInfo(mConfigPath).exists())
    mCfg = new QSettings(QDir::homePath() + "/.config/spark-store/config", QSettings::IniFormat,
                         this);
  else
  {
    mCfg = new QSettings(":/info/default_config", QSettings::IniFormat, this);
#if 0
    bool cfgDirOk;
    if(!qgetenv("SPARK_NO_INSTALL_CONFIG").toInt())
    {
      QString path = mConfigPath.section('/', 1, -2, QString::SectionIncludeLeadingSep);
      if(!QDir().exists(path))
      {
        if(!QDir().mkpath(path))
          sErrPop(QObject::tr("Config directory \"%1\" cannot be created.").arg(path));
        else
          cfgDirOk = true;
      }
      else
        cfgDirOk = true;

      if(cfgDirOk) // Only try copying if config dir is OK
        if(!QFile::copy(":/info/default_config", QDir::homePath() + "/.config/spark-store/config"))
          sErrPop(tr("Cannot install default config file!"));
    }
#endif
  }

  mNetMgr = new QNetworkAccessManager(this);
  mNetMgr->setProxy(QNetworkProxy(QNetworkProxy::NoProxy)); // FIXME
  mDistroName = SpkUtils::GetDistroName();

  // Initialize URL
  mApiRequestUrl = mCfg->value("url/api", "https://store.deepinos.org/api/").toString();
  mResourceRequestUrl = mCfg->value("url/res", "http://d.deepinos.org.cn/").toString();


  mUserAgentStr = QString("Spark-Store/%1 Distro/%2")
      .arg(GitVer::DescribeTags())
      .arg(mDistroName);

  // Finish all essential initialization before this.
  if(aCli)
    return;

  // UI Initialization
  mResMgr = new SpkResource(this); // Resource manager must be created before the windows
  SpkUi::Initialize();
  mMainWindow = new SpkMainWindow;
  SpkUi::Popup = new SpkUi::SpkPopup(mMainWindow);

  mMainWindow->show();
}

SpkStore::~SpkStore()
{
  delete mMainWindow;
  delete mLogger;
}

QNetworkReply *SpkStore::SendApiRequest(QString aPath, QJsonDocument aParam)
{
  QNetworkRequest request;
  request.setUrl(mApiRequestUrl + aPath);
  request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setHeader(QNetworkRequest::UserAgentHeader, mUserAgentStr);
  return mNetMgr->post(request, aParam.isEmpty() ? "{}" : aParam.toJson(QJsonDocument::Compact));
}

QNetworkReply *SpkStore::SendResourceRequest(QString aPath)
{
  QNetworkRequest request;
  request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
  request.setUrl(mResourceRequestUrl + aPath);
  request.setHeader(QNetworkRequest::UserAgentHeader, mUserAgentStr);
  return mNetMgr->get(request);
}

QNetworkReply *SpkStore::SendDownloadRequest(QUrl file, qint64 fromByte, qint64 toByte)
{
  QNetworkRequest request;
  request.setUrl(file);
  request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
  if(fromByte != -1 && toByte != -1)
  {
    request.setRawHeader("Range", QString("bytes=%1-%2").arg(fromByte).arg(toByte).toLocal8Bit());
  }
  request.setHeader(QNetworkRequest::UserAgentHeader, mUserAgentStr);
  return mNetMgr->get(request);
}

QNetworkReply *SpkStore::SendCustomHeadRequest(QNetworkRequest req)
{
  req.setHeader(QNetworkRequest::UserAgentHeader, mUserAgentStr);
  return mNetMgr->head(req);
}

static void InstallDefaultConfigs()
{
  //TODO:STUB
}
