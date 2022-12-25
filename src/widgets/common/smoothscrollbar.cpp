#include "smoothscrollbar.h"
#include <QWheelEvent>
#include <QDebug>
SmoothScrollBar::SmoothScrollBar(QWidget *parent) : QScrollBar(parent)
{
    m_scrollAni = new QPropertyAnimation(this);
    m_scrollAni->setTargetObject(this);
    m_scrollAni->setPropertyName("value");
    m_scrollAni->setEasingCurve(QEasingCurve::OutQuint); // 设置动画曲线，在Qt文档中有详细的介绍
    m_scrollAni->setDuration(500);                       // 设置动画时间，数值越小播放越快
    m_targetValue_v = value();                           // 将m_targetValue_v初始化
}

void SmoothScrollBar::setValue(int value)
{
    m_scrollAni->stop();                       // 停止现在的动画，防止出现冲突
    m_scrollAni->setStartValue(this->value()); // 设置动画滚动的初始值为当前位置
    m_scrollAni->setEndValue(value);           // 设置动画的结束位置为目标值
    m_scrollAni->start();                      // 开始动画
}
void SmoothScrollBar::scrollTop()
{
    setValue(-m_targetValue_v); // 开始动画
    m_targetValue_v = 0;
}
void SmoothScrollBar::scroll(int value)
{
    // 这里推荐评论区中大佬优化的写法
    if (m_targetValue_v - value >= 0)
    {
        m_targetValue_v -= value;  // 将目标值和相对位置进行运算
        setValue(m_targetValue_v); // 开始动画
    }
}

void SmoothScrollBar::mousePressEvent(QMouseEvent *e)
{
    // 当使用鼠标操作滚动条时，不会刷新m_targetValue_v的值，因而需要重写事件，对其进行刷新。
    m_scrollAni->stop();
    QScrollBar::mousePressEvent(e);
    m_targetValue_v = value();
}

void SmoothScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
    m_scrollAni->stop();
    QScrollBar::mouseReleaseEvent(e);
    m_targetValue_v = value();
}

void SmoothScrollBar::mouseMoveEvent(QMouseEvent *e)
{
    m_scrollAni->stop();
    QScrollBar::mouseMoveEvent(e);
    m_targetValue_v = value();
}
