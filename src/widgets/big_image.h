#ifndef BIG_IMAGE_H
#define BIG_IMAGE_H

#include <QMouseEvent>
#include <QLabel>

#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class big_image : public DBlurEffectWidget
{
    Q_OBJECT

public:
    explicit big_image(DBlurEffectWidget *parent = nullptr);

    QLabel *m_image;

    void setimage(QPixmap);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
};

#endif // BIG_IMAGE_H
