
#pragma once

#include <QObject>
#include <QQueue>
#include <QSemaphore>
#include <QMap>
#include <QNetworkReply>
#include <QVariant>

struct ResourceResult;
class SpkResourceContext;
class SpkPageBase;

class SpkResource : public QObject
{
  Q_OBJECT
  public:
    enum class ResourceType { HomeImage, AppIcon, TagIcon, AppScreenshot, };
    enum class ResourceStatus
    {
      Ready, ///< The resource is ready to be read now and the file link is included
      Deferred, ///< The resource is not in cache or outdated, and is being downloaded
      Failed, ///< The requested resource can not be downloaded and thus can not be loaded.
    };

  private:
    struct ResourceTask
    {
      QString pkgName, path;
      ResourceType type;
      QVariant info;
      int id;
    };

    static const QMap<ResourceType, QString> ResourceName;

  public:
    SpkResource(QObject *parent = nullptr);
    ResourceResult
    RequestResource(const int aId, const QString &aPkgName, ResourceType aType, const QString &aPath,
                    const QVariant &aInfo = 0);
    void PurgeCachedResource(const QString &aPkgName, SpkResource::ResourceType aType,
                             const QVariant &aInfo);

    inline QString GetCachePath(const ResourceTask &task);

    /**
     * @brief When the resource context was changed, the new context needs to acquire the resource
     *        manager, so the resource manager can download resource for the new context.
     *
     *        A resource context, in Spark Store, is likely to be a "page" of the UI. AppList is
     *        a page, Homepage is a page, AppDetails is a page. But each category is not a separated
     *        context; they're all from AppList page.
     * @param dest Pass a SpkResourceContext as the resource acquisition destination. Signals will
     *             be connected in this function internally. When a resource was downloaded, this
     *             object will be notified.
     * @param clearQueue determines if the awaiting tasks needs to be cleared.
     * @param stopOngoing determines if ongoing tasks needs to be terminated.
     */
    void Acquire(SpkPageBase *dest, bool stopOngoing, bool clearQueue = true);

  public:
    static SpkResource* Instance;
    const int mMaximumConcurrent; ///< Maximum number of concurrent resource downloads
    const QString mCacheDirectory;///< Where caches were stored

  private slots:
    void ResourceDownloaded();
    void TryBeginAwaitingTasks();

  private:
    ResourceResult LocateCachedResource(const ResourceTask &task);

  signals:
    void AcquisitionFinish(int id, ResourceResult result);

  private:
    // Operations to mAwaitingRequests and mWorkingRequests must be all made synchronously.
    // When connecting signal/slot pairs regarding operations to them, use QueuedConnection.
    QQueue<ResourceTask> mAwaitingRequests;
    QSemaphore *mRequestSemaphore;
    QMap<QNetworkReply*, int> mWorkingRequests;
};


struct ResourceResult
{
  SpkResource::ResourceStatus status;
  QByteArray data;
};

Q_DECLARE_METATYPE(ResourceResult);

/**
 * @brief SpkResourceContext is meant to be used by connecting signals from it. An object inheriting
 *        it would be theoretically a resource context. A resource context can acquire a SpkResource
 *        management object for downloading resources. More details at SpkResource::Acquire.
 */
