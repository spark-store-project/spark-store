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

    const QJsonObject obj = m_data.at(index.row()).toObject();

    switch (role) {
    case Qt::DisplayRole:
        return obj["name"].toString();
    case Qt::UserRole + 1: // 包名
        return obj["package"].toString();
    case Qt::UserRole + 2: // 当前版本
        return obj["current_version"].toString();
    case Qt::UserRole + 3: // 新版本
        return obj["new_version"].toString();
    case Qt::UserRole + 4: // 图标路径
        return obj["icon"].toString();
    default:
        return QVariant();
    }
}

void AppListModel::setUpdateData(const QJsonArray &data)
{
    beginResetModel();
    m_data = data;
    endResetModel();
}
