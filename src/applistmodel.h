#ifndef APPLISTMODEL_H
#define APPLISTMODEL_H

#include <QObject>

class AppListModel : public QObject
{
    Q_OBJECT
public:
    explicit AppListModel(QObject *parent = nullptr);

signals:
};

#endif // APPLISTMODEL_H
