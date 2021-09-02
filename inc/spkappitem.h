
#pragma once

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "qt/elidedlabel.h"

class SpkAppItem : public QWidget
{
    Q_OBJECT

  public:
    SpkAppItem(int appId, QWidget *parent = nullptr);
    void SetIcon(QPixmap p) { mIcon->setPixmap(p); }
    void SetTitle(QString s) { mTitle->setText(s); }
    void SetDescription(QString s) { mDescription->setText(s); }

  protected:
    void paintEvent(QPaintEvent *e);

  public:
    static constexpr int IconSize = 72;
    static constexpr QSize IconSize_ = { IconSize, IconSize };

  private:
    QLabel *mIcon;
    QLabel *mTitle;
    ElidedLabel *mDescription;
    int mAppId;

    QVBoxLayout *mLayText;
    QHBoxLayout *mMainLay;

};
