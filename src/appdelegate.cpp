#include "appdelegate.h"
#include <QIcon>
#include <QDebug>

AppDelegate::AppDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void AppDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    // 获取数据
    QString name = index.data(Qt::DisplayRole).toString();
    QString currentVersion = index.data(Qt::UserRole + 2).toString();
    QString newVersion = index.data(Qt::UserRole + 3).toString();
    QString iconPath = index.data(Qt::UserRole + 4).toString();
    // 获取包大小数据
    QVariant sizeVariant = index.data(Qt::UserRole + 5);
    QString size = sizeVariant.isValid() ? sizeVariant.toString() : "未知";

    // 绘制图标
    QIcon icon(iconPath);
    QRect iconRect(option.rect.x() + 10, option.rect.y() + 10, 32, 32);
    icon.paint(painter, iconRect);

    // 绘制文本，添加包大小信息
    QRect textRect(iconRect.right() + 10, option.rect.y() + 10, option.rect.width() - iconRect.width() - 20, option.rect.height() - 20);
    painter->drawText(textRect, Qt::TextWordWrap, QString("%1\n当前版本: %2 → 新版本: %3\n包大小: %4").arg(name, currentVersion, newVersion, size));
}

QSize AppDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(), 60); // 每行高度 60
}

bool AppDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        qDebug() << "点击了第" << index.row() << "行";
        return true;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}