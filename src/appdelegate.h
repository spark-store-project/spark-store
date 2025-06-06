#ifndef APPDELEGATE_H
#define APPDELEGATE_H

#include <QObject>

class AppDelegate : public QObject
{
    Q_OBJECT
public:
    explicit AppDelegate(QObject *parent = nullptr);

signals:
};

#endif // APPDELEGATE_H
