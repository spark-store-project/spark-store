#include "widgetanimation.h"

WidgetAnimation::WidgetAnimation()
{

}

void WidgetAnimation::widgetShake(QWidget *pWidget, int nRange)
{
    int nX = pWidget->x();
    int nY = pWidget->y();
    QPropertyAnimation *pAnimation = new QPropertyAnimation(pWidget,"geometry");
    pAnimation->setEasingCurve(QEasingCurve::InOutSine);
    pAnimation->setDuration(400);
    pAnimation->setStartValue(QRect(QPoint(nX,nY),pWidget->size()));

    int nShakeCount = 8;
    double nStep = 1.0/nShakeCount;
    for(int i = 1; i < nShakeCount; i++){
        nRange = i&1 ? -nRange : nRange;
        pAnimation->setKeyValueAt(nStep*i,QRect(QPoint(nX + nRange,nY),pWidget->size()));
    }

    pAnimation->setEndValue(QRect(QPoint(nX,nY),pWidget->size()));
    pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

QPropertyAnimation* WidgetAnimation::createWidgetOpacity(QWidget *pWidget, bool isAppear)
{
    QPropertyAnimation *animation = new QPropertyAnimation(pWidget, "windowOpacity", pWidget);
    //设置动画效果
    animation->setEasingCurve(QEasingCurve::Linear);
    //设置动画时间（单位：毫秒）
    animation->setDuration(500);
    // 设置动画步长值，以及在该位置时显示的透明度
    if(isAppear)
    {
        animation->setKeyValueAt(0, 0);
        // m_animation->setKeyValueAt(0.5, 0);
        animation->setKeyValueAt(1, 1);
    }else{
        animation->setKeyValueAt(0, 1);
        animation->setKeyValueAt(1, 0);
    }
    return animation;
}

void WidgetAnimation::widgetOpacity(QWidget *pWidget, bool isAppear)
{    
    // 开始动画
    createWidgetOpacity(pWidget, isAppear)->start();
}
