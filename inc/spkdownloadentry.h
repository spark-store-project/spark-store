
#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include "qt/elidedlabel.h"
#include "spkloading.h"
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>

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

  public slots:
    void SetBasicInfo(QString name, QPixmap icon);
    void SetStatus(DownloadEntryStatus status);
    void SetProgress(int);

  private:
    QLabel *mIcon, *mMessage;
    ElidedLabel *mAppName;
    QProgressBar *mProgress;
    QPushButton *mBtnDelete,
                *mBtnActions; // Actions include Retry Pause Install etc, one status at a time
    SpkLoading *mLoading;

    QHBoxLayout *mLayMsgs, *mLayMain;
    QVBoxLayout *mLayInfo;

};
