#ifndef SMOOTHLISTWIDGET_H
#define SMOOTHLISTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include "smoothscrollbar.h"
class SmoothListWidget: public QListWidget
{
    Q_OBJECT
public:
    explicit SmoothListWidget(QWidget *parent = nullptr);
    SmoothScrollBar* vScrollBar=new SmoothScrollBar(); //纵向滚动条
private:
    void wheelEvent(QWheelEvent* e); //捕获鼠标滚轮事件
signals:
    void msliderChanged(int p);
    void reachedBottom();

private slots:
    void onSliderChanged(int p);
};

#endif // SMOOTHLISTWIDGET_H
