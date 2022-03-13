
#pragma once

#include <QPainter>
#include <QScrollArea>
#include "spkwindow.h"

class ImgView : public QWidget
{
    Q_OBJECT

  public:
    ImgView(QWidget *parent = nullptr) : QWidget(parent) { mPixmap = nullptr; }
    void SetPixmap(QPixmap *p) { mPixmap = p; if(p) setFixedSize(p->size()); update(); }

  protected:
    void paintEvent(QPaintEvent *e)
    {
      QPainter p(this);
      if(mPixmap)
        p.drawPixmap(0, 0, *mPixmap);
      p.end();
      e->accept();
    }

  private:
    QPixmap *mPixmap;
};

class SpkImgViewer : public SpkWindow
{
    Q_OBJECT

  public:
    SpkImgViewer(QWidget *parent = nullptr);
    void ShowWithImage(int idx);
    void SetImageTotal(int a) { mTotalImg = a; }

  public slots:
    void Clear();
    void SetPixmap(int idx, QPixmap *img);

  private slots:
    void SwitchToImage(int idx);

  protected:
    bool event(QEvent*) override;

  private:
    void ResizeToFitImageSize(QSize);

  private:
    QPushButton *mBtnPrev, *mBtnNext;
    QScrollArea *mImgArea;
    QLabel *mImgIndict;
    QMap<int, QPixmap*> mImgMap;
    int mCurrentImg, mTotalImg;
    QPixmap mIconLoading;

    ImgView *mImgShow;
};
