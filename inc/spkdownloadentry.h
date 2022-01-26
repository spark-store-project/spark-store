
#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include "qt/elidedlabel.h"
#include "spkloading.h"
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTime>

class SpkDownloadEntry : public QWidget
{
    Q_OBJECT

  public:
    explicit SpkDownloadEntry(QWidget* parent = nullptr);
    ~SpkDownloadEntry();

    static constexpr QSize IconSize { 64, 64 };

    enum DownloadEntryStatus
    {
      Invalid = -1,
      Waiting,
      Downloading,
      Paused,
      Failed,
      ToBeInstalled,
      Installing,
      Installed,
      InstallFailed
    };

    void SetTotalBytes(qint64 total);
    void SetBasicInfo(QString name, QPixmap icon);
    void SetStatus(DownloadEntryStatus status, QString msg = "");
    void Progress(qint64 bytes);

  private:
    QLabel *mIcon, *mMessage;
    ElidedLabel *mAppName;
    QProgressBar *mProgress;
    QPushButton *mBtnDelete,
                *mBtnActions; // Actions include Retry Pause Install etc, one status at a time
    SpkLoading *mLoading;

    QHBoxLayout *mLayMsgs, *mLayMain;
    QVBoxLayout *mLayInfo;

    // Download status data
    qint64 mTotalBytes, mDownloadedBytes;
    QTime mLastReportTime;
    QString mReadableTotalSize;
};
