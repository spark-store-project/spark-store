
#pragma once

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class SpkAppItem : public QWidget
{
    Q_OBJECT
  public:
    SpkAppItem(QWidget *parent = nullptr);

    void SetIcon(QPixmap p) { mIcon->setPixmap(p); }
    void SetTitle(QString s) { mTitle->setText(s); }
    void SetDescription(QString s) { mDescription->setText(s); }

  protected:
    void paintEvent(QPaintEvent *e);

  private:
    QLabel *mIcon;
    QLabel *mTitle;
    QLabel *mDescription;

    QVBoxLayout *mLayText;
    QHBoxLayout *mMainLay;

    static constexpr int IconSize = 72;
};

