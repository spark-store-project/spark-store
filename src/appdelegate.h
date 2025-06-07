#ifndef APPDELEGATE_H
#define APPDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
// 添加 QEvent 头文件
#include <QEvent>

class AppDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit AppDelegate(QObject *parent = nullptr);

    // 重写方法
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

#endif // APPDELEGATE_H
