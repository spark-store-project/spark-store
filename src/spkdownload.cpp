
#include "spkdownload.h"
#include "spkutils.h"
#include "spkui_general.h"
#include "spkpopup.h"
#include <QEventLoop>
#include <QDir>

SpkDownloadMgr::SpkDownloadMgr(QObject *parent)
{
  mDestFolder = CFG->value("download/dir", QDir::homePath() + "/.local/spark-store/downloads")
                       .toString();

  QDir dest(mDestFolder);
  if(!dest.exists())
    QDir().mkdir(mDestFolder);

  // Distribution servers
  QString srvPaths = CFG->value("download/servers", "https://d.store.deepinos.org/").toString();
  mServers = srvPaths.split(";;");

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

  // Try get the file size first. If one server fails then go to next server
  RemoteFileInfo info;
  for(int i = 0; i < mServers.size() && info.Size == -1; i++)
  {
    info = GetRemoteFileInfo(mServers[i] + path);
    // TODO: Mark dead servers as unusable so they don't get scheduled first?
  }
  if(info.Size == -1) return false; // If all servers failed then we say it's a failure

  mCurrentRemoteFileInfo = info;
  mActiveWorkerCount = 0;

  // Create the destination file.
  mDestFile.close();
  mDestFile.setFileName(mDestFolder + '/' + SpkUtils::CutFileName(path));
  if(!mDestFile.open(QFile::ReadWrite))
    return false;

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
    mScheduledWorkers.append(worker);
  }

  // Link the worker's replies with the manager
  for(auto &i : mScheduledWorkers)
  {
    LinkReplyWithMe(i.Reply);
    i.Reply->setProperty("failCount", 0); // Used for fail retry algorithm
  }

  mProgressEmitterTimer.start();

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
    r->blockSignals(true);
    r->abort();
    r->deleteLater();
  }

  // Terminate and delete the temporary file
  mDestFile.close();
  if(!mDestFile.remove())
  {
    sErr(tr("SpkDownloadMgr: Cannot remove destination file %1 of a cancelled task")
           .arg(mDestFile.fileName()));
    SpkUi::Popup->Show(tr("The destination file of the cancelled task can't be deleted!"));
  }
  return false;
}

void SpkDownloadMgr::WorkerFinish()
{
  QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
  int id = reply->property("workerId").toInt();
  DownloadWorker &worker = mScheduledWorkers[id];

  mActiveWorkerCount--;

  if(reply->error() == QNetworkReply::NetworkError::NoError)
  {
    // Finished successfully, destroy associated stuff
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

      mProgressEmitterTimer.stop();
    }
  }
  else
  {
    // Failed here! Update our offset and required bytes count etc.
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
      return;
    }

    // We can still retry.
    worker.Reply =
        STORE->SendDownloadRequest(mServers[id] + mCurrentRemotePath,
                                   worker.BeginOffset,
                                   worker.BeginOffset + worker.BytesNeeded);
    LinkReplyWithMe(worker.Reply);
    worker.Reply->setProperty("failCount", reply->property("failCount").toInt() + 1);
    reply->deleteLater();
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
  mDownloadedBytes += replyData.size();
}

void SpkDownloadMgr::ProgressTimer()
{
  emit DownloadProgressed(mDownloadedBytes, mCurrentRemoteFileInfo.Size, mCurrentDownloadId);
}

void SpkDownloadMgr::LinkReplyWithMe(QNetworkReply *reply)
{
  mActiveWorkerCount++; // Each time you spin up a request you must do this so it's ok to do it here
  connect(reply, &QNetworkReply::readyRead, this, &SpkDownloadMgr::WorkerDownloadProgress);
  connect(reply, &QNetworkReply::finished, this, &SpkDownloadMgr::WorkerFinish);
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
    LinkReplyWithMe(mScheduledWorkers[i].Reply);
  }
}
