
#include <QPaintEvent>
#include <QPainter>
#include <QBrush>
#include "spknotifydot.h"
#include "spkstore.h"

/*
 * The font size and the actual geometry of this widget is hard coded
 * If you want to use this widget code please consider improving it
 */

SpkNotifyDot::SpkNotifyDot(QWidget *parent) :
  QLabel(parent)
{

}

void SpkNotifyDot::paintEvent(QPaintEvent *e)
{
  QPainter p(this);

  int h = height();

  p.setRenderHint(QPainter::Antialiasing);
  p.setBrush(QBrush(QColor(255, 70, 50)));
  p.setPen(Qt::transparent);
  p.drawEllipse(0, 0, h, h);
  p.setBrush(QBrush(QColor(Qt::white)));
  p.setPen(Qt::white);
  p.setFont(QFont("sansserif", 10, 1000));
  p.drawText(QRect(0, 0, h, h), Qt::AlignHCenter | Qt::AlignVCenter, text());

  p.end();
}
