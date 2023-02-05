#include "widgetanimation.h"
#include "widgets/base/basewidgetopacity.h"

void WidgetAnimation::widgetShake(QWidget *pWidget, int nRange)
{
    int nX = pWidget->x();
    int nY = pWidget->y();
    QPropertyAnimation *pAnimation = new QPropertyAnimation(pWidget, "geometry");
    pAnimation->setEasingCurve(QEasingCurve::InOutSine);
    pAnimation->setDuration(400);
    pAnimation->setStartValue(QRect(QPoint(nX, nY), pWidget->size()));

    int nShakeCount = 8;
    double nStep = 1.0 / nShakeCount;
    for (int i = 1; i < nShakeCount; i++)
    {
        nRange = i & 1 ? -nRange : nRange;
        pAnimation->setKeyValueAt(nStep * i, QRect(QPoint(nX + nRange, nY), pWidget->size()));
    }

    pAnimation->setEndValue(QRect(QPoint(nX, nY), pWidget->size()));
    pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

QPropertyAnimation *WidgetAnimation::createWidgetOpacity(QWidget *pWidget, bool isAppear)
{
    QPropertyAnimation *animation = new QPropertyAnimation(pWidget, "windowOpacity", pWidget);
    // 设置动画时间（单位：毫秒）
    animation->setDuration(500);
    if (isAppear)
    {
        // 设置动画效果
        animation->setEasingCurve(QEasingCurve::Linear);
        // 设置动画步长值，以及在该位置时显示的透明度（即动画关键帧）
        animation->setKeyValueAt(0.0, 0.0);
        animation->setKeyValueAt(1.0, 1.0);
    }
    else
    {
        animation->setEasingCurve(QEasingCurve::OutQuart);
        animation->setKeyValueAt(0.0, 1.0);
        animation->setKeyValueAt(1.0, 0.0);

        QObject::connect(animation, &QPropertyAnimation::finished, pWidget, [=]() { pWidget->close(); });
    }

    QObject::connect(animation, &QPropertyAnimation::valueChanged, pWidget, [=]()
            {
                pWidget->update(); // NOTE: 保证窗口透明度动画平滑
            });

    return animation;
}

void WidgetAnimation::widgetOpacity(QWidget *pWidget, bool isAppear)
{
    // 启动/关闭动画
    createWidgetOpacity(pWidget, isAppear)->start(QPropertyAnimation::DeleteWhenStopped);
}
