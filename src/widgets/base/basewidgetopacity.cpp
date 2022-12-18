#include "basewidgetopacity.h"

#include <QCloseEvent>
#include <QPropertyAnimation>

BaseWidgetOpacity::BaseWidgetOpacity(QWidget *parent) : DBlurEffectWidget(parent)
{
//    WidgetAnimation::widgetOpacity(this,true);
}

/// @brief 窗口关闭事件
/// @param event
void BaseWidgetOpacity::closeEvent(QCloseEvent *event)
{
    if (!closeWindowAnimation) {
        QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
        animation->setEasingCurve(QEasingCurve::OutQuart);
        animation->setDuration(500);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);

        QObject::connect(animation, &QPropertyAnimation::valueChanged, this, [=](const QVariant &value){
            this->update();
            // setWindowTitle(QString("ヾ(⌒∇⌒*)See You♪ - %1%").arg(int(value.toFloat() * 100)));
        });

        QObject::connect(animation, &QPropertyAnimation::finished, this, [=](){
            closeWindowAnimation = true;
            this->close();
        });

        animation->start();
        event->ignore();
    } else {
        event->accept();
    }
}
