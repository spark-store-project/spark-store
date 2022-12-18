#ifndef WIDGETANIMATION_H
#define WIDGETANIMATION_H

#include <QObject>
#include <QWidget>
#include <QPropertyAnimation>

class WidgetAnimation
{
public:
    WidgetAnimation();
    static void widgetShake(QWidget *pWidget, int nRange);

    static QPropertyAnimation* createWidgetOpacity(QWidget *pWidget, bool isAppear);
    static void widgetOpacity(QWidget *pWidget, bool isAppear);
};

#endif // WIDGETANIMATION_H
