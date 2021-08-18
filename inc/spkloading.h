
#pragma once

#include <QFrame>
#include <QTimeLine>

class SpkLoading : public QFrame
{
    Q_OBJECT
  public:
    explicit SpkLoading(QWidget *parent = nullptr);
    virtual void paintEvent(QPaintEvent *e) override;
    virtual void resizeEvent(QResizeEvent *e) override;

  private:
    QTimeLine *mAnimTimer;
    QList<int> mSizeList;
    int mUserHeight = 0;
    int dw, dh;
    double dx, dy;

  public slots:
    void start() { mAnimTimer->start(); }
    void stop() { mAnimTimer->stop(); }
    void Begin() { start(); setVisible(true); }
    void End() { stop(); setVisible(false); }
    void setHeight(int h) { mUserHeight = h; }
    void reset();

  private slots:
    void timer(int s);
    void loop();
};

