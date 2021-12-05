
#include <QMargins>
#include <QPainter>
#include <QVariant>
#include <QDebug>
#include "spkiconbutton.h"

SpkIconButton::SpkIconButton(QWidget *parent) :
  QPushButton(parent)
{

}

void SpkIconButton::SetIcon(QIcon i, QSize s)
{
  mPmapPaintedIcon = i.pixmap(s);

  setFixedSize((mPmapSize  = s.grownBy(QMargins(IconMargin, IconMargin, IconMargin, IconMargin))));
}

void SpkIconButton::SetIcon(QPixmap m)
{
  mPmapPaintedIcon = m;

  setFixedSize((mPmapSize = m.size().grownBy(QMargins(IconMargin, IconMargin,
                                                      IconMargin, IconMargin))));
}

void SpkIconButton::SetIconSize(QSize s)
{
  setFixedSize((mPmapSize = s.grownBy(QMargins(IconMargin, IconMargin, IconMargin, IconMargin))));
}

void SpkIconButton::paintEvent(QPaintEvent *e)
{
  QPushButton::paintEvent(e);

  // Paint the icon mask
  QPainter p(this), p1(&mPmapPaintedIcon);
  QBrush b(Qt::SolidPattern);

  p.drawPixmap(IconMargin, IconMargin, mPmapPaintedIcon);
  if(isDown() || isChecked())
  {
    b.setColor(SpkUi::ColorBtnMaskSelected);
  }
  else
  {
    b.setColor(SpkUi::ColorBtnMaskUnselected);
  }
  p1.setCompositionMode(QPainter::CompositionMode_SourceIn);
  p1.fillRect(0, 0, mPmapSize.width(), mPmapSize.height(), b);
  p1.end();
  p.drawPixmap(IconMargin, IconMargin, mPmapPaintedIcon);
  p.end();
}
