#include "basewidgetopacity.h"
#include "utils/widgetanimation.h"
#include "utils/utils.h"

#include <QSettings>
#include <QStandardPaths>
#include <QPropertyAnimation>
#include <QCloseEvent>

BaseWidgetOpacity::BaseWidgetOpacity(QWidget *parent) : DBlurEffectWidget(parent)
{
}

/**
 * @brief 窗口显示事件
 * @param event
 */
void BaseWidgetOpacity::showEvent(QShowEvent *event)
{
    // FIXME: wayland 不支持直接设置窗口透明度，需要调用 wayland 相关库（考虑抄控制中心“窗口移动时启用透明特效”代码？）
    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    bool isWayland = Utils::isWayland();
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

/**
 * @brief 窗口关闭事件
 * @param event
 */
void BaseWidgetOpacity::closeEvent(QCloseEvent *event)
{
    // FIXME: wayland 不支持直接设置窗口透明度，需要调用 wayland 相关库（考虑抄控制中心“窗口移动时启用透明特效”代码？）
    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    bool isWayland = Utils::isWayland();
    if (isWayland)
    {
        return DBlurEffectWidget::closeEvent(event);
    }

    if (!closeWindowAnimation)
    {
        closeWindowAnimation = true;
        WidgetAnimation::widgetOpacity(this, false);

        event->ignore();
    }
    else
    {
        event->accept();
    }
}
