
#include "spkstretchlayout.h"

SpkStretchLayout::SpkStretchLayout(QWidget *parent) : QLayout(parent)
{
}

SpkStretchLayout::~SpkStretchLayout()
{
  QLayoutItem *item;
  while((item = takeAt(0)))
    delete item;
}

void SpkStretchLayout::addItem(QLayoutItem *item)
{
  mItems.append(item);
}

QSize SpkStretchLayout::sizeHint() const
{
  int n = mItems.count();
  auto minSize = mItems.isEmpty() ? QSize { 0, 0 } : mItems.first()->minimumSize();
  QSize s { 200, 50 }; // s: Hint
  if(n)
    s = s.expandedTo(QSize { minSize.width(), minSize.height() * n });
  return s + n * QSize(spacing(), spacing());
}

QSize SpkStretchLayout::minimumSize() const
{
  int n = mItems.count();
  auto minSize = mItems.isEmpty() ? QSize { 0, 0 } : mItems.first()->minimumSize();
  QSize s(minSize.width(), minSize.height() * n);
  return s + n * QSize(spacing(), spacing());

}

int SpkStretchLayout::count() const
{
  return mItems.size();
}

QLayoutItem *SpkStretchLayout::itemAt(int i) const
{
  return mItems.value(i);
}

QLayoutItem *SpkStretchLayout::takeAt(int i)
{
  return i >=0 && i < mItems.size() ? mItems.takeAt(i) : nullptr;
}

void SpkStretchLayout::setGeometry(const QRect &rect)
{
  QLayout::setGeometry(rect);
  if(mItems.isEmpty())
    return;

  int spc = spacing(), w = rect.width() - spc;
  QSize size;
  auto itm = mItems.first();

  // All items are considered the same, so we only calculate with the first item.
  // Figure out how many at most can we squeeze into one line
  int countPerLine = w / (itm->minimumSize().width() + spacing());

  if(countPerLine >= mItems.size()) // All items fit in one line
    size = itm->minimumSize();
  // Won't fit in one line.
  else // Stretch items.
    size = QSize {(w / countPerLine - spc), itm->maximumSize().height() };

  QLayoutItem *o;
  for(int i = 0; i < mItems.size(); i++)
  {
    o = mItems.at(i);
    QRect geo;
    geo.setSize(size);
    geo.moveTo((i % countPerLine) * (size.width() + spc) + spc,
               (i / countPerLine) * (size.height() + spacing()) + spc);
    o->setGeometry(geo);
  }
}
