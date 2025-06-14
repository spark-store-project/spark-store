#include "customlabel.h"
#include <QGuiApplication>

CustomLabel::CustomLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
}

QPixmap CustomLabel::pixmap() const
{
    const QPixmap *p = QLabel::pixmap();
    return p ? *p : QPixmap();
}

void CustomLabel::setPixmap(const QPixmap &pixmap)
{
    QPixmap _pixmap = pixmap;
    _pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
    _pixmap = _pixmap.scaled(size() * _pixmap.devicePixelRatio(),
                             Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);

    QLabel::setPixmap(_pixmap);
}
