#ifndef CUSTOMLABEL_H
#define CUSTOMLABEL_H

#include <QLabel>

class CustomLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)

public:
    explicit CustomLabel(QWidget *parent = nullptr,
                         Qt::WindowFlags f = Qt::WindowFlags());

    virtual QPixmap pixmap() const;
    virtual void setPixmap(const QPixmap &pixmap);
};

#endif // CUSTOMLABEL_H
