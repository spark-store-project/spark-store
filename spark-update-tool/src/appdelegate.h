#pragma once

#include <QStyledItemDelegate>
#include <QHash>
#include <QQueue>
#include <QProcess>
#include <QElapsedTimer>
#include "downloadmanager.h"

struct DownloadInfo {
    int progress = 0;
    bool isDownloading = false;
    bool isInstalled = false;
    bool isInstalling = false; // 新增：标记是否正在安装
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
    void startDownloadForAll();

signals:
    void updateDisplay(const QString &packageName);

private:
    DownloadManager *m_downloadManager;
    QHash<QString, DownloadInfo> m_downloads;
    QAbstractItemModel *m_model = nullptr;

    QQueue<QString> m_installQueue;
    bool m_isInstalling = false;
    QProcess *m_installProcess = nullptr;
    QString m_installingPackage; // 当前正在安装的包名
    QElapsedTimer m_spinnerTimer; // 用于转圈动画

    void enqueueInstall(const QString &packageName);
    void startNextInstall();
};
