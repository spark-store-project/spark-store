#include "smoothlistwidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QWheelEvent>
#include <QDebug>
SmoothListWidget::SmoothListWidget(QWidget *parent) : QListWidget(parent)
{
    vScrollBar->setOrientation(Qt::Orientation::Vertical); // 将滚动条设置为纵向
    setVerticalScrollBar(vScrollBar);                      // 设置纵向滚动条
    connect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged(int)));
}
void SmoothListWidget::wheelEvent(QWheelEvent *e)
{
    // 当捕获到事件后，调用相对滚动的槽函数
    vScrollBar->scroll(e->angleDelta().y());
}
void SmoothListWidget::onSliderChanged(int p)
{
    int startRow = count();
    if (p == vScrollBar->maximum())
    {
        emit reachedBottom(); // 1
    }
    emit msliderChanged(p); // 2
}
