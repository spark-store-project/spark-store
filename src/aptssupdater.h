#ifndef APTSSUPDATER_H
#define APTSSUPDATER_H

#include <QWidget>
#include <QStringList>

class aptssUpdater : public QWidget
{
    Q_OBJECT
public:
    explicit aptssUpdater(QWidget *parent = nullptr);

    QStringList getUpdateablePackages(); // 查询可更新包列表及更新内容
    qint64 getTotalDownloadSize();       // 获取更新总大小预估
    QStringList getPackageSizes();       // 获取每个包的大小
signals:
};

#endif // APTSSUPDATER_H