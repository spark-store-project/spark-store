#include "appdelegate.h"
#include <QIcon>
#include <QDebug>

AppDelegate::AppDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void AppDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    // 绘制背景
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, QColor("#F3F4F6")); 
    }

    // 设置字体
    QFont boldFont = option.font;
    boldFont.setBold(true);

    QFont normalFont = option.font;

    // 数据
    QString name = index.data(Qt::DisplayRole).toString();
    QString currentVersion = index.data(Qt::UserRole + 2).toString();
    QString newVersion = index.data(Qt::UserRole + 3).toString();
    QString iconPath = index.data(Qt::UserRole + 4).toString();
    QString size = index.data(Qt::UserRole + 5).toString();
    QString description = index.data(Qt::UserRole + 6).toString(); // 假设额外说明文本

    // 区域定义
    QRect rect = option.rect;
    int margin = 10;
    int spacing = 6;
    int iconSize = 40;

    // 图标
    QRect iconRect(rect.left() + margin, rect.top() + (rect.height() - iconSize) / 2, iconSize, iconSize);
    QIcon(iconPath).paint(painter, iconRect);

    // 文本起点
    int textX = iconRect.right() + margin;
    int textWidth = rect.width() - textX - 100; // 留出按钮区域

    // 绘制应用名称
    QRect nameRect(textX, rect.top() + margin, textWidth, 20);
    painter->setFont(boldFont);
    painter->setPen(QColor("#333333")); // 改为深色，确保清晰
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, name); // 绘制应用名称

    // 绘制版本信息
    QRect versionRect(textX, nameRect.bottom() + spacing, textWidth, 20);
    painter->setFont(normalFont);
    painter->setPen(QColor("#888888")); // 使用浅灰色，保持对比
    painter->drawText(versionRect, Qt::AlignLeft | Qt::AlignVCenter,
                      QString("当前版本: %1 → 新版本: %2").arg(currentVersion, newVersion));

    // 描述行
    QRect descRect(textX, versionRect.bottom() + spacing, textWidth, 40);
    painter->setFont(normalFont);
    painter->setPen(QColor("#AAAAAA"));
    painter->drawText(descRect, Qt::TextWordWrap,
                    //   QString("更新说明：%1\n包大小：%2").arg(description, size));
                    QString("包大小：%1").arg(size));

    // 更新按钮（样式占位）
    QRect buttonRect(rect.right() - 80, rect.top() + (rect.height() - 30) / 2, 70, 30);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#267AFF"));
    painter->drawRoundedRect(buttonRect, 6, 6);
    painter->setPen(Qt::white);
    painter->drawText(buttonRect, Qt::AlignCenter, "更新");

    painter->restore();
}


QSize AppDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(), 110); // 每行高度 110
}

bool AppDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        qDebug() << "点击了第" << index.row() << "行";
        return true;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}