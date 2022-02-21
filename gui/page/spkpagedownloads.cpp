

#include "page/spkpagedownloads.h"
#include "pkgs/spkpkgmgrbase.h"
#include "spkuimsg.h"
#include "spkutils.h"

SpkUi::SpkPageDownloads::SpkPageDownloads(QWidget *parent) :
  SpkPageBase(parent)
{
  mMainLay = new QVBoxLayout(this);
  mLayEntries = new QVBoxLayout;
  mScrollWidget = new QWidget;
  mScrollArea = new QScrollArea(this);

  mLayEntries->setAlignment(Qt::AlignTop);
  mScrollWidget->setLayout(mLayEntries);
  mScrollArea->setWidget(mScrollWidget);
  mScrollArea->setWidgetResizable(true);
  mMainLay->addWidget(mScrollArea);
  setLayout(mMainLay);

  mDownloadMgr = new SpkDownloadMgr(this);
  connect(mDownloadMgr, &SpkDownloadMgr::DownloadProgressed,
          this, &SpkPageDownloads::DownloadProgress);

  mNextDownloadId = 0;
  mCurrentStatus = Idle;

  connect(mDownloadMgr, &SpkDownloadMgr::DownloadStopped,
          this, &SpkPageDownloads::DownloadStopped, Qt::QueuedConnection);
  connect(PKG, &SpkPkgMgrBase::ReportInstallResult,
          this, &SpkPageDownloads::InstallationEnded);
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
  auto id = mNextDownloadId;
  QPixmap icon;
  if(iconData.status != SpkResource::ResourceStatus::Ready || !icon.loadFromData(iconData.data))
    icon.load(":/icons/broken-icon.svg");
  entry->SetBasicInfo(name, icon, mDownloadMgr->GetDestFilePath(path));
  entry->SetStatus(SpkDownloadEntry::Waiting);
  entry->setProperty("entryId", id);
  entry->setProperty("path", path);

  mNextDownloadId++;

  mEntries[id] = entry;
  mLayEntries->addWidget(entry);

  connect(entry, &SpkDownloadEntry::Action,
          this, &SpkPageDownloads::EntryAction);

  NewDownloadTask(id, path);
}

void SpkUi::SpkPageDownloads::DownloadStopped(SpkDownloadMgr::TaskResult status, int id)
{
  switch(status)
  {
    case SpkDownloadMgr::Success:
      mEntries[id]->SetStatus(SpkDownloadEntry::ToBeInstalled);
      break;

    case SpkDownloadMgr::FailCannotCreateFile:
      mEntries[id]->SetStatus(SpkDownloadEntry::DownloadFailed,
                              tr("Cannot create download file. Download failed."));
      break;

    case SpkDownloadMgr::FailNoVaibleServer:
      mEntries[id]->SetStatus(SpkDownloadEntry::DownloadFailed,
                              tr("Connection unstable or server failure. Download failed."));
      break;

    case SpkDownloadMgr::FailCancel:
      mEntries[id]->SetStatus(SpkDownloadEntry::DownloadFailed,
                              tr("This download was cancelled."));
      break;

    case SpkDownloadMgr::Fail:
      mEntries[id]->SetStatus(SpkDownloadEntry::DownloadFailed,
                              tr("Unknown error. Download failed."));
      break;
  }

  if(status == SpkDownloadMgr::Success)
    SpkUiMessage::SendDesktopNotification(
          tr("App \"%1\" downloaded, and ready to install.").arg(mEntries[id]->GetTaskName()));
  else if(status != SpkDownloadMgr::FailCancel)
    SpkUiMessage::SendDesktopNotification(
          tr("Error occurred downloading \"%1\".").arg(mEntries[id]->GetTaskName()));


  // Continue next download task
  if(!mWaitingDownloads.isEmpty())
  {
    auto nextTask = mWaitingDownloads.dequeue();
    auto nextEntry = mEntries[nextTask.first];
    nextEntry->SetStatus(SpkDownloadEntry::Starting);
    mDownloadMgr->StartNewDownload(nextTask.second, nextTask.first);
    mCurrentStatus = Waiting;
  }
  else
  {
    mCurrentStatus = Idle;
  }
}

void SpkUi::SpkPageDownloads::EntryAction(SpkDownloadEntry::EntryAction act)
{
  SpkDownloadEntry *entry = static_cast<SpkDownloadEntry*>(sender());
  auto id = entry->property("entryId").toInt();
  switch(act)
  {
    case SpkDownloadEntry::AbortDownload:
      mDownloadMgr->CancelCurrentDownload(); // Only one task at a time so simply abort download
      break;

    case SpkDownloadEntry::RetryDownload:
      mLayEntries->removeWidget(entry); // Move to list tail
      mLayEntries->addWidget(entry);
      NewDownloadTask(id, entry->property("path").toString());
      entry->SetStatus(SpkDownloadEntry::Waiting);
      break;

    case SpkDownloadEntry::StartInstall:
      switch(PKG->ExecuteInstallation(entry->GetFilePath(), id))
      {
        case SpkPkgMgrBase::Succeeded:
          entry->SetStatus(SpkDownloadEntry::Installing);
          break;

        case SpkPkgMgrBase::Failed:
          entry->SetStatus(SpkDownloadEntry::InstallFailed,
                           tr("Failed to start installation."));
          break;

        default: break;
      }

      break;

    case SpkDownloadEntry::RemoveEntry:
      mLayEntries->removeWidget(entry);
      mEntries.remove(id);
      for(auto i = mWaitingDownloads.begin(); i != mWaitingDownloads.end(); i++)
      {
        if(i->first == id)
        {
          mWaitingDownloads.erase(i);
          break;
        }
      }
      entry->setVisible(false);
      entry->deleteLater();
      break;
  }
}

void SpkUi::SpkPageDownloads::InstallationEnded(int id,
                                                SpkPkgMgrBase::PkgInstallResult result,
                                                int exitCode)
{
  if(result == SpkPkgMgrBase::Succeeded)
  {
    mEntries[id]->SetStatus(SpkDownloadEntry::Installed);
  }
  else
  {
    mEntries[id]->SetStatus(SpkDownloadEntry::InstallFailed,
                            tr("Install failed, exit code: %1.").arg(exitCode));
  }
}

void SpkUi::SpkPageDownloads::NewDownloadTask(int id, QString downloadPath)
{
  if(mCurrentStatus != Idle)
    mWaitingDownloads.enqueue({ id, downloadPath }); // Queue download task for future
  else
  {
    auto nextEntry = mEntries[id];
    nextEntry->SetStatus(SpkDownloadEntry::Starting);
    mCurrentStatus = Waiting;
    if(!mDownloadMgr->StartNewDownload(downloadPath, id)) // Initiate a download task when idle
    {
      // If fails to start then try next one. Emitting this signal causes
      // SpkPageDownloads::DownloadStopped to be activated and thus tries next item in queue
      emit mDownloadMgr->DownloadStopped(SpkDownloadMgr::FailNoVaibleServer, id);
    }
  }
}
