

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
  setLayout(mMainLay);
}

SpkUi::SpkPageDownloads::~SpkPageDownloads()
{
  // TODO
}

void SpkUi::SpkPageDownloads::DownloadProgress(qint64 downloadedBytes, qint64 totalBytes, int id)
{
  // TODO
}
