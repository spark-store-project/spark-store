#ifndef APPLISTMODEL_H
#define APPLISTMODEL_H

#include <QAbstractListModel>
#include <QJsonArray>
// 添加 QJsonObject 头文件
#include <QJsonObject>
#include <QDebug>
class AppListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AppListModel(QObject *parent = nullptr);

    // 重写方法
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // 设置更新数据
    void setUpdateData(const QJsonArray &data);
    
    // 获取忽略状态
    bool isAppIgnored(const QModelIndex &index) const;

private:
    QList<QVariantMap> m_data; // 修改类型为 QList<QVariantMap>
};

#endif // APPLISTMODEL_H