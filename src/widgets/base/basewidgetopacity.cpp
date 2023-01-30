#include "basewidgetopacity.h"
#include "utils/widgetanimation.h"

#include <QSettings>
#include <QStandardPaths>
#include <QPropertyAnimation>
#include <QCloseEvent>

BaseWidgetOpacity::BaseWidgetOpacity(QWidget *parent) : DBlurEffectWidget(parent)
{
    //    WidgetAnimation::widgetOpacity(this,true);
}

/**
 * @brief 窗口显示事件
 * @param event
 */
void BaseWidgetOpacity::showEvent(QShowEvent *event)
{
    // FIXME: wayland 不支持直接设置窗口透明度，需要调用 wayland 相关库（考虑抄控制中心“窗口移动时启用透明特效”代码？）
    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    bool isWayland = config.value("build/isWayland").toBool();
    if (!isWayland)
    {
        if (!showWindowAnimation)
        {
            showWindowAnimation = true;
            WidgetAnimation::widgetOpacity(this, true);
        }
    }

    DBlurEffectWidget::showEvent(event);
}

/// @brief 窗口关闭事件
/// @param event
void BaseWidgetOpacity::closeEvent(QCloseEvent *event)
{
    // FIXME: wayland 不支持直接设置窗口透明度，需要调用 wayland 相关库（考虑抄控制中心“窗口移动时启用透明特效”代码？）
    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    bool isWayland = config.value("build/isWayland").toBool();
    if (isWayland)
    {
        return DBlurEffectWidget::closeEvent(event);
    }

    if (!closeWindowAnimation)
    {
        closeWindowAnimation = true;

        QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
        animation->setEasingCurve(QEasingCurve::OutQuart);
        animation->setDuration(500);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);

        QObject::connect(animation, &QPropertyAnimation::valueChanged, this, [=](const QVariant &value)
                         {
                             this->update();
                             // setWindowTitle(QString("ヾ(⌒∇⌒*)See You♪ - %1%").arg(int(value.toFloat() * 100)));
                         });

        QObject::connect(animation, &QPropertyAnimation::finished, this, [=]()
                         { this->close(); });

        animation->start();
        event->ignore();
    }
    else
    {
        event->accept();
    }
}
