
#include <QPainter>
#include <QStyleOption>
#include "spkappitem.h"

SpkAppItem::SpkAppItem(QWidget *parent) : QWidget(parent)
{
  mMainLay = new QHBoxLayout(this);
  mLayText = new QVBoxLayout;

  mIcon = new QLabel;
  mIcon->setFixedSize(IconSize, IconSize);
  mIcon->setAutoFillBackground(false);

  mTitle = new QLabel;
  mTitle->setWordWrap(false);
  mTitle->setObjectName("styAppItmTitle");
  mTitle->setAutoFillBackground(true);
  mDescription = new QLabel;
  mDescription->setWordWrap(true);
  mDescription->setObjectName("styAppItmDesc");
  mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  mDescription->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  mDescription->setAutoFillBackground(true);
  mLayText->addWidget(mTitle);
  mLayText->addWidget(mDescription);
  mLayText->setAlignment(Qt::AlignTop);

  mMainLay->addWidget(mIcon);
  mMainLay->addLayout(mLayText);

  setMinimumHeight(82);
  setMaximumHeight(82);
  setMinimumWidth(300);
  setMaximumWidth(350);
}

void SpkAppItem::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
