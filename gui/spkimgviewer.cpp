
#include "spktitlebar.h"
#include "spkimgviewer.h"
#include "spkui_general.h"
#include <QDebug>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QScreen>

SpkImgViewer::SpkImgViewer(QWidget *parent) :
  SpkWindow(parent),
  mIconLoading(QIcon(":/icons/loading-icon.svg").pixmap({ 72, 72 }))
{
  mImgIndict = new QLabel;
  mImgIndict->setText("%1/%2");
  mBtnPrev = new QPushButton;
  mBtnPrev->setText("<");
  mBtnNext = new QPushButton;
  mBtnNext->setText(">");

  auto titleBar = GetTitleBar();
  titleBar->SetUseIcon(false);
  titleBar->SetTitle(tr("Image Preview"));
  titleBar->SetOperationButton(SpkTitleBar::OperationButton::Close);

  auto lay = titleBar->GetUserSpace();
  lay->setAlignment(Qt::AlignVCenter);
  lay->addStretch();
  lay->addWidget(mBtnPrev);
  lay->addWidget(mImgIndict);
  lay->addWidget(mBtnNext);
  lay->addStretch();

  mImgArea = new QScrollArea;
  mImgArea->setWidgetResizable(true);
  mImgArea->setContentsMargins(10, 10, 10, 10);

  mImgShow = new ImgView;

  mImgArea->setWidget(mImgShow);

  auto w = new QWidget;
  auto l = new QHBoxLayout;
  l->setContentsMargins(10, 10, 10, 10);
  l->addWidget(mImgArea);
  w->setLayout(l);
  SetCentralWidget(w);

  connect(mBtnPrev, &QPushButton::clicked,
          [&](){ if(mCurrentImg > 0) { SwitchToImage(--mCurrentImg); } });
  connect(mBtnNext, &QPushButton::clicked,
          [&](){ if(mCurrentImg < mTotalImg) { SwitchToImage(++mCurrentImg); } });
}

void SpkImgViewer::ShowWithImage(int idx)
{
  SwitchToImage(idx);
  show();
}

void SpkImgViewer::Clear()
{
  mImgMap.clear();
  mImgShow->SetPixmap(nullptr);
  mCurrentImg = 1;
}

void SpkImgViewer::SetPixmap(int idx, QPixmap *img)
{
  mImgMap[idx] = img;
  if(mCurrentImg == idx)
  {
    mImgShow->SetPixmap(img);
  }
  ResizeToFitImageSize(img->size());
}

void SpkImgViewer::SwitchToImage(int idx)
{
  auto img = mImgMap.value(idx, nullptr);
  mCurrentImg = idx;
  mImgShow->SetPixmap(img ? img : &mIconLoading);
  if(img)
    ResizeToFitImageSize(img->size());
  if(idx == 1)
  {
    mBtnPrev->setEnabled(false);
    mBtnNext->setEnabled(true);
  }
  else if(idx == mTotalImg)
  {
    mBtnPrev->setEnabled(true);
    mBtnNext->setEnabled(false);
  }
  else
  {
    mBtnPrev->setEnabled(true);
    mBtnNext->setEnabled(true);
  }
  mImgIndict->setText(QString("%1/%2").arg(mCurrentImg).arg(mTotalImg));
}

bool SpkImgViewer::event(QEvent *e)
{
  if(e->type() == QEvent::WindowDeactivate)
    close();
  return SpkWindow::event(e);
}

void SpkImgViewer::ResizeToFitImageSize(QSize s)
{
  auto targetSize = s;
  targetSize.rheight() += SpkTitleBar::Height;
  targetSize = s.grownBy(QMargins(10, 10, 10, 10));
  targetSize = targetSize.boundedTo(SpkUi::PrimaryScreenSize * 0.8);
  resize(targetSize);
  targetSize /= 2;
  auto targetPos = SpkUi::PrimaryScreenSize / 2 - targetSize;
  move(targetPos.width(), targetPos.height());
}
