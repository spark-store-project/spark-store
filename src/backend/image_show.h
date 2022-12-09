#ifndef IMAGE_SHOW_H
#define IMAGE_SHOW_H

#include <QWidget>
#include <QMouseEvent>

#include "widgets/big_image.h"

class image_show : public QWidget
{
    Q_OBJECT

public:
    explicit image_show(QWidget *parent = nullptr);

    int desktop_w;
    int desktop_h;

    void setImage(QPixmap);
    ~image_show();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    big_image *m_dialog;
    QLabel *m_label;
    QLabel image;
    QPixmap m_image;
};

#endif // IMAGE_SHOW_H
