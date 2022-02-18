
#include "spkdownload.h"
#include "spkutils.h"
#include "spkui_general.h"
#include "spkpopup.h"
#include <QEventLoop>
#include <QDir>
#include <QApplication>
#include <QDebug>

SpkDownloadMgr::SpkDownloadMgr(QObject *parent)
{
  CFG->BindField("dirs/download", &mDestFolder,
                 QString("*/.local/spark-store/downloads"));
  mDestFolder.replace('*', QDir::homePath());

  QDir dest(mDestFolder);
  if(!dest.exists())
    QDir().mkdir(mDestFolder);

  // Distribution servers

  CFG->BindField("download/servers", &mBulkServerPaths,
                 "https://d1.store.deepinos.org.cn/;;"
                 "https://d2.store.deepinos.org.cn/;;"
                 "https://d3.store.deepinos.org.cn/;;"
                 "https://d4.store.deepinos.org.cn/;;"
                 "https://d5.store.deepinos.org.cn/",
                 std::bind(&SpkDownloadMgr::ServerAddressesChangedCallback, this));

  mServers = mBulkServerPaths.split(";;");

  mCurrentDownloadId = -1;
  mActiveWorkerCount = 0;
  mDownloadedBytes = 0;

  mProgressEmitterTimer.setInterval(800);

  connect(&mProgressEmitterTimer, &QTimer::timeout,
          this, &SpkDownloadMgr::ProgressTimer);
}

SpkDownloadMgr::~SpkDownloadMgr()
{
  // TODO
}

void SpkDownloadMgr::SetNewServers(QList<QString> servers)
{
  mServers = servers;
}

SpkDownloadMgr::RemoteFileInfo SpkDownloadMgr::GetRemoteFileInfo(QUrl url)
{
  QEventLoop event;
  QNetworkRequest request;
  request.setUrl(QUrl(url));

  // Use a HEAD request to get file's actual size.
  request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
  QNetworkReply *reply = STORE->SendCustomHeadRequest(request);
  connect(reply, &QNetworkReply::finished, &event, &QEventLoop::quit);
  event.exec();

  RemoteFileInfo ret;
  if(reply->hasRawHeader("Content-Length"))
    ret.Size = reply->header(QNetworkRequest::ContentLengthHeader).toUInt();
  ret.SupportPartialDownload = (reply->rawHeader("Accept-Ranges") == QByteArrayLiteral("bytes"));

  reply->deleteLater();
  return ret;
}

void SpkDownloadMgr::SetDestinationFolder(QString path)
{
  QDir dir(path);
  if(!dir.exists())
  {
    if(!QDir().mkdir(path))
    {
      sErrPop(tr("Cannot create download destination folder %1! It is now /tmp/.").arg(path));
      mDestFolder = "/tmp/";
      return;
    }
  }
  mDestFolder = path;
}

bool SpkDownloadMgr::StartNewDownload(QString path, int downloadId)
{  
  if(mCurrentDownloadId != -1)
    return false; // Already downloading something

  // Reserve the manager and let everyone else wait
  mCurrentDownloadId = downloadId;

  // Try get the file size first. If one server fails then go to next server
  qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
  RemoteFileInfo info;
  for(int i = 0; i < mServers.size() && info.Size == -1; i++)
  {
    info = GetRemoteFileInfo(mServers[i] + path);
    // TODO: Mark dead servers as unusable so they don't get scheduled first?
  }
  qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
  if(info.Size == -1)
  {
    sNotify(tr("Server request failure, %1 cannot be downloaded.").arg(SpkUtils::CutFileName(path)));
    sErr(tr("SpkDownloadMgr: all start requests failed for %1.").arg(path));
    return false; // If all servers failed then we say it's a failure
  }

  mCurrentRemoteFileInfo = info;
  mActiveWorkerCount = 0;

  // Create the destination file.
  if(mDestFile.isOpen())
    mDestFile.close();
  if(!SpkUtils::EnsureDirExists(mDestFolder))
  {
    sNotify(tr("Cannot create download destination folder, download cannot start."));
    sErr(tr("SpkDownloadMgr: Download directory %1 cannot be created.").arg(mDestFolder));
    return false;
  }
  mDestFile.setFileName(mDestFolder + '/' + SpkUtils::CutFileName(path));
  if(!mDestFile.open(QFile::ReadWrite))
  {
    sNotify(tr("Cannot write to destination file, download cannot start."));
    sErr(tr("SpkDownloadMgr: Download destination file %1 cannot be opened with mode %2.")
         .arg(mDestFile.fileName())
         .arg(mDestFile.openMode()));
    return false;
  }

  mCurrentRemotePath = path;

  // Schedule tasks onto different servers if it's supported
  if(info.SupportPartialDownload && mServers.size() > 1)
  {
    int blockSize = info.Size / mServers.size();
    int i;
    for(i = 0; i < mServers.size() - 1; i++)
    {
      DownloadWorker worker
      {
        .BeginOffset = i * blockSize,
        .BytesNeeded = blockSize,
        .BytesRecvd = 0
      };
      worker.Reply =
          STORE->SendDownloadRequest(mServers[i] + path,
                                     worker.BeginOffset,
                                     worker.BeginOffset + worker.BytesNeeded);
      worker.Reply->setProperty("workerId", i);
      mScheduledWorkers.append(worker);
    }
    // Last one
    DownloadWorker worker
    {
      .BeginOffset = i * blockSize,
      .BytesNeeded = info.Size - i * blockSize,
      .BytesRecvd = 0
    };
    worker.Reply =
        STORE->SendDownloadRequest(mServers[i] + path,
                                   worker.BeginOffset,
                                   worker.BeginOffset + worker.BytesNeeded);
    worker.Reply->setProperty("workerId", i);
    mScheduledWorkers.append(worker);
  }
  else
  {
    DownloadWorker worker { .BeginOffset = 0, .BytesNeeded = info.Size, .BytesRecvd = 0 };
    worker.Reply = STORE->SendDownloadRequest(mServers[0] + path);
    worker.Reply->setProperty("workerId", mScheduledWorkers.size());
    mScheduledWorkers.append(worker);
  }

  // Link the worker's replies with the manager
  for(auto &i : mScheduledWorkers)
  {
    LinkReplyWithMe(i.Reply);
    i.Reply->setProperty("failCount", 0); // Used for fail retry algorithm
    mActiveWorkerCount++;
  }

  mProgressEmitterTimer.start();

  mCurrentDownloadId = downloadId;

  return true;
}

bool SpkDownloadMgr::PauseCurrentDownload()
{
  // UNIMPLEMENTED
  return false;
}

bool SpkDownloadMgr::CancelCurrentDownload()
{
  // Don't proceed when no downloads are there
  if(mCurrentDownloadId == -1)
    return false;

  // Terminate all workers
  for(auto &i : mScheduledWorkers)
  {
    auto r = i.Reply;
    if(!r) // Don't bother with finished workers
      continue;
    r->blockSignals(true);
    r->abort();
    r->deleteLater();
  }

  // Terminate and delete the temporary file
  mDestFile.close();
  if(!mDestFile.remove())
  {
    sWarn(tr("SpkDownloadMgr: Cannot remove destination file %1 of a cancelled task")
           .arg(mDestFile.fileName()));
    sNotify(tr("The destination file of the cancelled task can't be deleted!"));
  }

  // Tell the UI to schedule next task and cleanup current status
  emit DownloadStopped(FailCancel, mCurrentDownloadId);
  mScheduledWorkers.clear();
  mFailureRetryQueue.clear();
  mCurrentDownloadId = -1;
  mDownloadedBytes = 0;
  mProgressEmitterTimer.stop();

  return true;
}

void SpkDownloadMgr::WorkerFinish()
{
  QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
  int id = reply->property("workerId").toInt();
  DownloadWorker &worker = mScheduledWorkers[id];

  mActiveWorkerCount--;

  if(reply->error() == QNetworkReply::NetworkError::NoError)
  {
    // Finished successfully, write any data possibly left in buffer
    auto replyData = reply->readAll();
    mDestFile.seek(worker.BeginOffset + worker.BytesRecvd);
    mDestFile.write(replyData);
    worker.BytesRecvd += replyData.size();
    worker.Watchdog = 0;
    mDownloadedBytes += replyData.size();

    // Destroy associated stuff
    reply->deleteLater();
    worker.Reply = nullptr;

    // Try schedule fail retries here.
    TryScheduleFailureRetries(id);

    // Check if we're finished.
    if(mActiveWorkerCount == 0)
    {
      emit DownloadStopped(Success, mCurrentDownloadId);
      mScheduledWorkers.clear();
      mFailureRetryQueue.clear();
      mCurrentDownloadId = -1;
      mDownloadedBytes = 0;
      mDestFile.close(); // Remember to close file!!

      mProgressEmitterTimer.stop();
    }
  }
  else
  {
    ProcessWorkerError(worker, id);
  }
}

void SpkDownloadMgr::WorkerDownloadProgress()
{
  QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
  DownloadWorker &worker = mScheduledWorkers[reply->property("workerId").toInt()];
  auto replyData = reply->readAll();

  mDestFile.seek(worker.BeginOffset + worker.BytesRecvd);
  mDestFile.write(replyData);
  worker.BytesRecvd += replyData.size();
  worker.Watchdog = 0;
  mDownloadedBytes += replyData.size();
}

void SpkDownloadMgr::WorkerError(QNetworkReply::NetworkError err)
{
  QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
  int id = reply->property("workerId").toInt();
  DownloadWorker &worker = mScheduledWorkers[id];

  switch(err)
  {
    case QNetworkReply::TimeoutError:
      sNotify(tr("A download has timed out, retrying..."));
      sWarn(tr("SpkDownloadMgr: %1 download timed out. Retrying").arg(reply->url().toString()));
      break;

    default:
      sNotify(tr("An error occured when downloading, retrying..."));
      sWarn(tr("SpkDownloadMgr: %1 fails with error %2. Retrying")
            .arg(reply->url().toString())
            .arg((int)err));
      break;
  }

  ProcessWorkerError(worker, id);
}

void SpkDownloadMgr::ProgressTimer()
{
  // Check watchdog
  for(auto &i : mScheduledWorkers)
  {
    if(i.Reply == nullptr) continue;
    if(++i.Watchdog > WatchDogMaximum)
    {
      // This reply has timed out
      emit i.Reply->errorOccurred(QNetworkReply::TimeoutError);
      // FIXME: Likely a Qt Bug. If you add these two lines (which you should if it works!),
      // And the target reply is a reply created on retry, then the signal on the last line
      // is NEVER EMITTED.
//      i.Reply->blockSignals(true);
//      i.Reply->abort();
    }
  }
  emit DownloadProgressed(mDownloadedBytes, mCurrentRemoteFileInfo.Size, mCurrentDownloadId);
}

void SpkDownloadMgr::ProcessWorkerError(DownloadWorker &worker, int id)
{
  auto reply = worker.Reply;

  // Update our offset and required bytes count etc.
  worker.BeginOffset += worker.BytesRecvd;
  worker.BytesNeeded -= worker.BytesRecvd;
  worker.BytesRecvd = 0;

  if(reply->property("failCount").toInt() > MaximumThreadRetryCount)
  {
    // Failed too many times, this server is probably down or really bad condition.
    // Schedule it on other servers.
    reply->deleteLater();
    worker.Reply = nullptr;
    mFailureRetryQueue.enqueue(worker);
    if(mActiveWorkerCount < mScheduledWorkers.size())
      TryScheduleFailureRetries();
    return;
  }

  // We can still retry.
  worker.Reply =
      STORE->SendDownloadRequest(reply->url(),
                                 worker.BeginOffset,
                                 worker.BeginOffset + worker.BytesNeeded);
  LinkReplyWithMe(worker.Reply);
  worker.Reply->setProperty("failCount", reply->property("failCount").toInt() + 1);
  worker.Reply->setProperty("workerId", id);
  worker.Watchdog = 0;
  reply->deleteLater();
}

void SpkDownloadMgr::LinkReplyWithMe(QNetworkReply *reply)
{
  connect(reply, &QNetworkReply::readyRead, this, &SpkDownloadMgr::WorkerDownloadProgress);
  connect(reply, &QNetworkReply::finished, this, &SpkDownloadMgr::WorkerFinish);
  connect(reply, &QNetworkReply::errorOccurred, this, &SpkDownloadMgr::WorkerError);
  reply->setProperty("linked", 1);
}

void SpkDownloadMgr::TryScheduleFailureRetries()
{
  if(mFailureRetryQueue.isEmpty())
    return;

  for(int i = 0; i < mScheduledWorkers.size() && !mFailureRetryQueue.isEmpty(); i++)
  {
    TryScheduleFailureRetries(i);
  }
}

void SpkDownloadMgr::TryScheduleFailureRetries(int i)
{
  if(mFailureRetryQueue.isEmpty())
    return;
  if(mScheduledWorkers[i].Reply == nullptr)
  {
    // Schedule it here, it has finished its job
    mScheduledWorkers[i] = mFailureRetryQueue.dequeue();
    DownloadWorker &worker = mScheduledWorkers[i];
    mScheduledWorkers[i].Reply =
        STORE->SendDownloadRequest(mServers[i] + mCurrentRemotePath,
                                   worker.BeginOffset,
                                   worker.BeginOffset + worker.BytesNeeded);
    mScheduledWorkers[i].Reply->setProperty("workerId", i);
    LinkReplyWithMe(mScheduledWorkers[i].Reply);
  }
}

bool SpkDownloadMgr::ServerAddressesChangedCallback()
{
  if(mCurrentDownloadId != -1)
    return false;

  // URL format verification *is done in the GUI*, we jsut have to split it here
  mServers = mBulkServerPaths.split(";;");
  return true;
}
