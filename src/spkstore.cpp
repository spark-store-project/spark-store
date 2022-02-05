
#include <spkui_general.h>
#include <QPluginLoader>
#include <QDir>
#include <QApplication>
#include <QtNetwork/QNetworkProxy>

#include "dtk/spkdtkplugin.h"
#include "gitver.h"
#include "spkmainwindow.h"
#include "spkpopup.h"
#include "spkstore.h"
#include "spkutils.h"

SpkStore *SpkStore::Instance = nullptr;
static bool InstallDefaultConfigs(QString configPath);

SpkStore::SpkStore(bool aCli, QString &aLogPath)
{
  mLogger = new SpkLogger(aLogPath);

  // Singleton
  Q_ASSERT(Instance == nullptr);
  Instance = this;

  // Finish all essential initialization after this.

  mConfigPath = QDir::homePath() + "/.config/spark-store/config"; //TODO: flexible config via CLI
  if(QFileInfo::exists(mConfigPath))
    mCfg = new SpkConfig(this, QDir::homePath() + "/.config/spark-store/config");
  else
  {
#if 1
    if(InstallDefaultConfigs(mConfigPath))
      mCfg = new SpkConfig(this, QDir::homePath() + "/.config/spark-store/config");
    else
#endif
      mCfg = new SpkConfig(this, ":/info/default_config");
  }

  mNetMgr = new QNetworkAccessManager(this);
  mNetMgr->setProxy(QNetworkProxy(QNetworkProxy::NoProxy)); // FIXME
  mDistroName = SpkUtils::GetDistroName();

  // Initialize URL
  mCfg->BindField("url/api", &mApiRequestUrl, "https://store.deepinos.org/api/");
  mCfg->BindField("url/res", &mResourceRequestUrl, "http://img.store.deepinos.org.cn/");

  mUserAgentStr = QString("Spark-Store/%1 Distro/%2")
      .arg(GitVer::DescribeTags())
#ifdef NDEBUG
      .arg(mDistroName);
#else
      .arg(mDistroName + " SparkDeveloper");
#endif

  // Finish all essential initialization before this.
  if(aCli)
    return;

  // UI Initialization
  mResMgr = new SpkResource(this); // Resource manager must be created before the windows
  SpkUi::Initialize();
  SpkUi::SpkUiMetaObject.SetAccentColor(QColor(200,100,0));
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

static bool InstallDefaultConfigs(QString configPath)
{
  bool cfgDirOk = false;
  if(!qEnvironmentVariableIntValue("SPARK_NO_INSTALL_CONFIG"))
  {
    cfgDirOk = SpkUtils::EnsureDirExists(SpkUtils::CutPath(configPath));

    if(cfgDirOk) // Only try copying if config dir is OK
    {
      if(!QFile::copy(":/info/default_config", QDir::homePath() + "/.config/spark-store/config"))
      {
        sErrPop(QObject::tr("Cannot install default config file!"));
        return false;
      }
      else
      {
        // Copying from resource to disk causes the file to be read only (444)
        // Set it to 644 manually
        QFile::setPermissions(QDir::homePath() + "/.config/spark-store/config",
                              QFileDevice::WriteOwner | QFileDevice::ReadOwner |
                              QFileDevice::ReadGroup | QFileDevice::ReadOther);
        return true;
      }
    }
    else
      return false;
  }
  return false;
}
