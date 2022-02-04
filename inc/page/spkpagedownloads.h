#ifndef SPKPAGEDOWNLOADS_H
#define SPKPAGEDOWNLOADS_H

#include "spkdownload.h"
#include "spkdownloadentry.h"
#include "page/spkpagebase.h"

namespace SpkUi
{
  class SpkPageDownloads : public SpkPageBase
  {
      Q_OBJECT
    public:
      SpkPageDownloads(QWidget *parent = nullptr);
      ~SpkPageDownloads();

    public slots:
      void AddDownloadTask(QString name, QString pkgName, QString path);

    private:
      // Logic
      SpkDownloadMgr *mDownloadMgr;
      QMap<uint, SpkDownloadEntry*> mEntries;
      uint mNextDownloadId;
      QQueue<QPair<uint, QString>> mWaitingDownloads;
      enum { Idle, Waiting, Downloading } mCurrentStatus;

      // UI
      QVBoxLayout *mLayEntries, *mMainLay;
      QWidget *mScrollWidget;
      QScrollArea *mScrollArea;

    private slots:
      void DownloadProgress(qint64 downloadedBytes, qint64 totalBytes, int id);
      void DownloadStopped(SpkDownloadMgr::TaskResult status, int id);
  };
}

#endif // SPKPAGEDOWNLOADS_H
