
#pragma once

#include <QScrollArea>
#include <QVBoxLayout>
#include <QFutureWatcher>
#include <QMutex>
#include "page/spkpagebase.h"
#include "ui_settings.h"

namespace SpkUi
{
  class SpkPageSettings : public SpkPageBase
  {
      Q_OBJECT

    public:
      SpkPageSettings(QWidget *parent = nullptr);
      ~SpkPageSettings();

      void SetupUi();
      void ReadConfiguration();
      void SaveConfiguration();

      void CountCleaning();

      virtual void Activated() override;

    private slots:
      void on_btnCleanDownloadedContent_clicked();
      void on_btnCleanResourceCache_clicked();
      void on_btnViewDownloadedContent_clicked();
      void on_btnViewResourceCache_clicked();

      void CountFinishResource();
      void CountFinishDownload();
      void CleanedResource();
      void CleanedDownload();

    private:
      QScrollArea *mMainArea;
      QVBoxLayout *mMainLay;
      QWidget *mSettingsWidget;
      Ui::SpkUiSettings *ui;

      QString mRepoListUrl;

      QFutureWatcher<void>  mFwResourceCount,
                            mFwDownloadCount,
                            mFwResourceClean,
                            mFwDownloadClean;
      QMutex mMutResource, mMutDownload;
      int64_t mBytesResource, mBytesDownloads;

  };
}
