
#pragma once

#include <QLabel>

class SpkNotifyDot : public QLabel
{
    Q_OBJECT

  public:
    SpkNotifyDot(QWidget *parent = nullptr);
    ~SpkNotifyDot() {}

  protected:
    virtual void paintEvent(QPaintEvent *) override;
//    virtual int widthForHeight(int h) override;
};
