
#include "spkdownloadentry.h"
#include "spklogging.h"

constexpr QSize SpkDownloadEntry::IconSize;

SpkDownloadEntry::SpkDownloadEntry(QWidget *parent)
{
  mIcon = new QLabel;
  mAppName = new ElidedLabel;
  mMessage = new QLabel;
  mProgress = new QProgressBar;
  mLoading = new SpkLoading;
  mBtnDelete = new QPushButton;
  mBtnActions = new QPushButton;

  mLayInfo = new QVBoxLayout;
  mLayMsgs = new QHBoxLayout;
  mLayMain = new QHBoxLayout;

  mLoading->setVisible(false);
  mIcon->setFixedSize(IconSize);
  mProgress->setRange(0, 1000);

  mLayMsgs->addWidget(mAppName);
  mLayMsgs->addStretch();
  mLayMsgs->addWidget(mMessage);

  mLayInfo->addLayout(mLayMsgs);
  mLayInfo->addWidget(mProgress);
  mLayInfo->setAlignment(Qt::AlignVCenter);

  mLayMain->addWidget(mIcon);
  mLayMain->addLayout(mLayInfo);
  mLayMain->addWidget(mLoading);
  mLayMain->addWidget(mBtnActions);
  mLayMain->addWidget(mBtnDelete);

  setLayout(mLayMain);
}


SpkDownloadEntry::~SpkDownloadEntry()
{
  // TODO
}

void SpkDownloadEntry::SetBasicInfo(QString name, QPixmap icon)
{
  mAppName->setText(name);
  mIcon->setPixmap(icon.scaled(IconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void SpkDownloadEntry::SetStatus(DownloadEntryStatus status)
{
  switch(status)
  {
    case Waiting:
      mMessage->setText(tr("Waiting for download"));
      mProgress->setVisible(false);
      mBtnActions->setVisible(false);
      mBtnDelete->setVisible(true);
      break;

    case Downloading:
      mMessage->setText(tr(""));
      mProgress->setVisible(true);
      mBtnActions->setVisible(true);
      break;

    case Paused:
      mMessage->setText(tr("Paused"));
      break;

    case Failed:
      mMessage->setText(tr("Download Failed"));
      mProgress->setVisible(false);
      break;

    case ToBeInstalled:
      mMessage->setText(tr("Download Finished"));
      mProgress->setVisible(false);
      break;

    case Installing:
      mMessage->setText("");
      mProgress->setVisible(false);
      mLoading->setVisible(true);
      mLoading->Begin();

    case Installed:
      mMessage->setText(tr("Installed"));
      mLoading->End();
      mLoading->setVisible(false);
      break;

    case InstallFailed:
      mMessage->setText(tr("Install Failed"));
      mLoading->End();
      mLoading->setVisible(false);
      break;

    case Invalid:
      break;
  }
}

void SpkDownloadEntry::SetProgress(int p)
{
  mProgress->setValue(p);
}
