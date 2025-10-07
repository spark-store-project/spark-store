#include "applistmodel.h"

AppListModel::AppListModel(QObject *parent) : QAbstractListModel(parent) {}

int AppListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_data.size();
}

QVariant AppListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.size())
        return QVariant();

    const QVariantMap &map = m_data.at(index.row()); // 直接访问 QVariantMap
    switch (role) {
    case Qt::DisplayRole:
        return map.value("name");
    case Qt::UserRole + 1: // 包名
        return map.value("package");
    case Qt::UserRole + 2: // 当前版本
        return map.value("current_version");
    case Qt::UserRole + 3: // 新版本
        return map.value("new_version");
    case Qt::UserRole + 4: // 图标路径
        return map.value("icon");
    case Qt::UserRole + 5: // 文件大小
        return map.value("size");
    case Qt::UserRole + 6: // 描述
        return map.value("description");
    case Qt::UserRole + 7: // 下载 URL
        return map.value("download_url"); // 返回下载 URL
    case Qt::UserRole + 8: // 忽略状态
        return map.value("ignored");
    default:
        return QVariant();
    }
}

void AppListModel::setUpdateData(const QJsonArray &updateInfo)
{
    beginResetModel();
    m_data.clear(); // 清空 QList<QVariantMap>

    for (const auto &item : updateInfo) {
        QJsonObject obj = item.toObject();
        QVariantMap map;
        map["package"] = obj["package"].toString();
        map["name"] = obj["name"].toString();
        map["current_version"] = obj["current_version"].toString();
        map["new_version"] = obj["new_version"].toString();
        map["icon"] = obj["icon"].toString();
        map["size"] = obj["size"].toString();
        map["download_url"] = obj["download_url"].toString(); // 确保设置下载 URL
        map["ignored"] = obj["ignored"].toBool(); // 设置忽略状态
        m_data.append(map); // 添加到 QList<QVariantMap>

        qDebug() << "设置到模型的包名:" << map["package"].toString() << "忽略状态:" << map["ignored"].toBool();
        qDebug() << "设置到模型的下载 URL:" << map["download_url"].toString(); // 检查设置的数据
    }

    endResetModel();
}

bool AppListModel::isAppIgnored(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= m_data.size())
        return false;
    
    const QVariantMap &map = m_data.at(index.row());
    return map.value("ignored").toBool();
}