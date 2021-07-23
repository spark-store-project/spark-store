
#include <QPaintEvent>
#include <QPainter>
#include "spkloading.h"
#include "spkui_general.h"

SpkLoading::SpkLoading(QWidget *parent) : QWidget(parent)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  mAnimTimer = new QTimeLine(400, this);
  mAnimTimer->setFrameRange(10, 30);
  mAnimTimer->setEasingCurve(QEasingCurve::InCubic);
  for(int i = 0; i < 5; i++)
    mSizeList.append(20);
  connect(mAnimTimer, &QTimeLine::frameChanged, this, &SpkLoading::timer);
  connect(mAnimTimer, &QTimeLine::finished, this, &SpkLoading::loop);
}

void SpkLoading::paintEvent(QPaintEvent *e)
{
  QPainter p(this);
  QPen pen(Qt::NoPen);
  QBrush b(SpkUi::CurrentColorSet[SpkUi::AccentColor], Qt::SolidPattern);
  p.setBrush(b);
  p.setPen(pen);
  p.setRenderHint(QPainter::Antialiasing);
  dx = width() / 2 - dh * 2;
  dy = height() / 2;

  double r;
  for(int i = 0; i < 5; i++)
  {
    r = dh * mSizeList[i] / 80;
    p.drawEllipse({ dx, dy }, r, r);
    dx += dh;
  }
  e->accept();
}

void SpkLoading::resizeEvent(QResizeEvent *e)
{
  // Calculate size of drawing space
  if(mUserHeight != 0 && mUserHeight * 5 <= e->size().width())
  {
    dw = 5 * mUserHeight;
    dh = mUserHeight;
    return;
  }
  dh = e->size().height();
  if(width() < dh * 5)
  {
    dw = e->size().width();
    dh = dw / 5;
  }
  else
    dw = dh * 5;
}

void SpkLoading::timer(int s)
{
  for(int i = 4; i > 0; i--)
    mSizeList[i] = mSizeList[i - 1];
  mSizeList[0] = s;
  update();
}

void SpkLoading::loop()
{
  mAnimTimer->setDirection(mAnimTimer->direction() == QTimeLine::Forward ?
                             QTimeLine::Backward : QTimeLine::Forward);
  mAnimTimer->start();
}

void SpkLoading::reset()
{
  for(int i = 0; i < 5; i++)
    mSizeList[i] = 20;
  update();
}
