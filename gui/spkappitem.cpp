
#include <QPainter>
#include <QStyleOption>
#include "spkappitem.h"
#include "qt/elidedlabel.h"

const QSize SpkAppItem::IconSize_;

SpkAppItem::SpkAppItem(int appId, QWidget *parent) : QWidget(parent)
{
  mAppId = appId;

  mMainLay = new QHBoxLayout(this);
  mLayText = new QVBoxLayout;

  mIcon = new QLabel;
  mIcon->setFixedSize(IconSize, IconSize);
  mIcon->setAutoFillBackground(false);

  // NOTE: Make mTitle ElidedTitle too?
  mTitle = new QLabel;
  mTitle->setWordWrap(false);
  mTitle->setObjectName("styAppItmTitle");
  mTitle->setAutoFillBackground(true);
  mDescription = new ElidedLabel;
//  mDescription->setWordWrap(true); // Commented out since ElidedLabel lacks of these methods
  mDescription->setObjectName("styAppItmDesc");
  mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//  mDescription->setAlignment(Qt::AlignTop | Qt::AlignLeft);
//  mDescription->setAutoFillBackground(true);
  mLayText->addWidget(mTitle);
  mLayText->addWidget(mDescription);
  mLayText->setAlignment(Qt::AlignTop);

  mMainLay->addWidget(mIcon);
  mMainLay->addLayout(mLayText);

  setMinimumHeight(82);
  setMaximumHeight(82);
  setMinimumWidth(300);
//  setMaximumWidth(350);
}

void SpkAppItem::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
