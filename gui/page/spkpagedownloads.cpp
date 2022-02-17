

#include "page/spkpagedownloads.h"
#include "spkutils.h"

SpkUi::SpkPageDownloads::SpkPageDownloads(QWidget *parent) :
  SpkPageBase(parent)
{
  mMainLay = new QVBoxLayout(this);
  mLayEntries = new QVBoxLayout;
  mScrollWidget = new QWidget;
  mScrollArea = new QScrollArea(this);

  mScrollWidget->setLayout(mLayEntries);
  mScrollArea->setWidget(mScrollWidget);
  mScrollArea->setWidgetResizable(true);
  mMainLay->addWidget(mScrollArea);
  mMainLay->addStretch();
  setLayout(mMainLay);

  mDownloadMgr = new SpkDownloadMgr(this);
  connect(mDownloadMgr, &SpkDownloadMgr::DownloadProgressed,
          this, &SpkPageDownloads::DownloadProgress);

  mNextDownloadId = 0;
  mCurrentStatus = Idle;

  connect(mDownloadMgr, &SpkDownloadMgr::DownloadStopped,
          this, &SpkPageDownloads::DownloadStopped, Qt::QueuedConnection);
}

SpkUi::SpkPageDownloads::~SpkPageDownloads()
{
  // TODO
}

void SpkUi::SpkPageDownloads::DownloadProgress(qint64 downloadedBytes, qint64 totalBytes, int id)
{
  if(!totalBytes)
    return;

  if(mCurrentStatus == Waiting && totalBytes)
  {
    mCurrentStatus = Downloading;
    mEntries[id]->SetTotalBytes(totalBytes);
    mEntries[id]->SetStatus(SpkDownloadEntry::Downloading);
  }
  mEntries[id]->Progress(downloadedBytes);
}

void SpkUi::SpkPageDownloads::AddDownloadTask(QString name, QString pkgName, QString path)
{
  // Add a new download entry into the UI
  auto entry = new SpkDownloadEntry;
  auto iconData = RES->CacheLookup(pkgName, SpkResource::ResourceType::AppIcon, 0);
  QPixmap icon;
  if(iconData.status != SpkResource::ResourceStatus::Ready || !icon.loadFromData(iconData.data))
    icon.load(":/icons/broken-icon.svg");
  entry->SetBasicInfo(name, icon);
  entry->SetStatus(SpkDownloadEntry::Waiting);
  auto id = mNextDownloadId;

  mNextDownloadId++;

  mEntries[id] = entry;
  mLayEntries->addWidget(entry);

  if(mCurrentStatus != Idle)
    mWaitingDownloads.enqueue({ id, path }); // Queue download task for future
  else
  {
    mCurrentStatus = Waiting;
    if(!mDownloadMgr->StartNewDownload(path, id)) // Initiate a download task when idle
    {
      // If fails to start then try next one. Emitting this signal causes
      // SpkPageDownloads::DownloadStopped to be activated and thus tries next item in queue
      emit mDownloadMgr->DownloadStopped(SpkDownloadMgr::FailNoVaibleServer, id);
    }
  }

}

void SpkUi::SpkPageDownloads::DownloadStopped(SpkDownloadMgr::TaskResult status, int id)
{
  switch(status)
  {
    case SpkDownloadMgr::Success:
      mEntries[id]->SetStatus(SpkDownloadEntry::ToBeInstalled);
      break;

    case SpkDownloadMgr::FailCannotCreateFile:
      mEntries[id]->SetStatus(SpkDownloadEntry::Failed,
                              tr("Cannot create download file. Download failed."));
      break;

    case SpkDownloadMgr::FailNoVaibleServer:
      mEntries[id]->SetStatus(SpkDownloadEntry::Failed,
                              tr("Connection unstable or server failure. Download failed."));
      break;

    case SpkDownloadMgr::Fail:
      mEntries[id]->SetStatus(SpkDownloadEntry::Failed,
                              tr("Unknown error. Download failed."));
      break;
  }

  // Continue next download task
  if(!mWaitingDownloads.isEmpty())
  {
    auto nextTask = mWaitingDownloads.dequeue();
    mDownloadMgr->StartNewDownload(nextTask.second, nextTask.first);
    mCurrentStatus = Waiting;
  }
  else
  {
    mCurrentStatus = Idle;
  }
}
