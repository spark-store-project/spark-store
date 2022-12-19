#ifndef BASEWIDGETOPACITY_H
#define BASEWIDGETOPACITY_H

#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class BaseWidgetOpacity : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit BaseWidgetOpacity(QWidget *parent = nullptr);

signals:

public slots:

    // QWidget interface
protected:
    bool closeWindowAnimation = false;
    void closeEvent(QCloseEvent *event) override;
};

#endif // BASEWIDGETOPACITY_H
