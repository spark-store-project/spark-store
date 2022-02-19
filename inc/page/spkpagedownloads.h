#ifndef SPKPAGEDOWNLOADS_H
#define SPKPAGEDOWNLOADS_H

#include "spkdownload.h"
#include "spkdownloadentry.h"
#include "page/spkpagebase.h"
#include "pkgs/spkpkgmgrbase.h"

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
      QQueue<QPair<int, QString>> mWaitingDownloads;
      enum { Idle, Waiting, Downloading } mCurrentStatus;

      // UI
      QVBoxLayout *mLayEntries, *mMainLay;
      QWidget *mScrollWidget;
      QScrollArea *mScrollArea;

    private slots:
      void DownloadProgress(qint64 downloadedBytes, qint64 totalBytes, int id);
      void DownloadStopped(SpkDownloadMgr::TaskResult status, int id);
      void EntryAction(SpkDownloadEntry::EntryAction);
      void InstallationEnded(int id, SpkPkgMgrBase::PkgInstallResult, int exitCode);

    private:
      void NewDownloadTask(int id, QString downloadPath);
  };
}

#endif // SPKPAGEDOWNLOADS_H
