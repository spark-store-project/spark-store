#ifndef BASEWIDGETOPACITY_H
#define BASEWIDGETOPACITY_H

#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class BaseWidgetOpacity : public DBlurEffectWidget
{
    Q_OBJECT

public:
    explicit BaseWidgetOpacity(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

protected:
    bool showWindowAnimation = false;
    bool closeWindowAnimation = false;
};

#endif // BASEWIDGETOPACITY_H
