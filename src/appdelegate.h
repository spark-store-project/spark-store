#pragma once

#include <QStyledItemDelegate>
#include <QHash>
#include "downloadmanager.h"

struct DownloadInfo {
    int progress = 0;
    bool isDownloading = false;
};

class AppDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit AppDelegate(QObject *parent = nullptr);

    void setModel(QAbstractItemModel *model);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    void updateDisplay(const QString &packageName);

private:
    DownloadManager *m_downloadManager;
    QHash<QString, DownloadInfo> m_downloads;
    QAbstractItemModel *m_model = nullptr;
};
