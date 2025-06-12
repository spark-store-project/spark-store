#ifndef APPDELEGATE_H
#define APPDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent> 
// 前向声明 DownloadManager 类
class DownloadManager; 

class AppDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit AppDelegate(QObject *parent = nullptr);

    // 重写方法
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    void updateDisplay(); // 声明更新显示的信号

private:
    struct DownloadInfo {
        int progress = 0;
        bool isDownloading = false;
    };
    QHash<QString, DownloadInfo> m_downloads; // 使用包名作为键的下载状态
    DownloadManager *m_downloadManager;

};

#endif // APPDELEGATE_H
