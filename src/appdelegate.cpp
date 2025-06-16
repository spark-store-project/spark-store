#include "appdelegate.h"
#include <QIcon>
#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QProgressBar>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>

AppDelegate::AppDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_downloadManager(new DownloadManager(this)) {

    connect(m_downloadManager, &DownloadManager::downloadFinished, this,
            [this](const QString &packageName, bool success) {
        if (m_downloads.contains(packageName)) {
            m_downloads[packageName].isDownloading = false;
            emit updateDisplay(packageName);
            qDebug() << (success ? "下载完成:" : "下载失败:") << packageName;
        }
    });

    connect(m_downloadManager, &DownloadManager::downloadProgress, this,
            [this](const QString &packageName, int progress) {
        if (m_downloads.contains(packageName)) {
            m_downloads[packageName].progress = progress;
            qDebug()<<progress;
            emit updateDisplay(packageName); // 实时刷新进度条
        }
    });
}

void AppDelegate::setModel(QAbstractItemModel *model) {
    m_model = model;
}

void AppDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    else
        painter->fillRect(option.rect, QColor("#F3F4F6"));

    QFont boldFont = option.font;
    boldFont.setBold(true);
    QFont normalFont = option.font;

    QString name = index.data(Qt::DisplayRole).toString();
    QString currentVersion = index.data(Qt::UserRole + 2).toString();
    QString newVersion = index.data(Qt::UserRole + 3).toString();
    QString iconPath = index.data(Qt::UserRole + 4).toString();
    QString size = index.data(Qt::UserRole + 5).toString();
    QString description = index.data(Qt::UserRole + 6).toString();

    QRect rect = option.rect;
    int margin = 10, spacing = 6, iconSize = 40;

    QRect iconRect(rect.left() + margin, rect.top() + (rect.height() - iconSize) / 2, iconSize, iconSize);
    QIcon(iconPath).paint(painter, iconRect);

    int textX = iconRect.right() + margin;
    int textWidth = rect.width() - textX - 100;

    QRect nameRect(textX, rect.top() + margin, textWidth, 20);
    painter->setFont(boldFont);
    painter->setPen(QColor("#333333"));
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, name);

    QRect versionRect(textX, nameRect.bottom() + spacing, textWidth, 20);
    painter->setFont(normalFont);
    painter->setPen(QColor("#888888"));
    painter->drawText(versionRect, Qt::AlignLeft | Qt::AlignVCenter,
                      QString("当前版本: %1 → 新版本: %2").arg(currentVersion, newVersion));

    QRect descRect(textX, versionRect.bottom() + spacing, textWidth, 40);
    painter->setFont(normalFont);
    painter->setPen(QColor("#AAAAAA"));
    painter->drawText(descRect, Qt::TextWordWrap,
                      QString("包大小：%1 MB").arg(QString::number(size.toDouble() / (1024 * 1024), 'f', 2)));

    QString packageName = index.data(Qt::UserRole + 1).toString();
    bool isDownloading = m_downloads.contains(packageName) && m_downloads[packageName].isDownloading;
    int progress = m_downloads.value(packageName, DownloadInfo{0, false}).progress;

    if (isDownloading) {
        QRect progressRect(rect.right() - 270, rect.top() + (rect.height() - 20) / 2, 150, 20);
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = progressRect;
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = progress;
        progressBarOption.text = QString("%1%").arg(progress);
        progressBarOption.textVisible = true;
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
        
        // 修改后的取消按钮绘制代码
        QRect buttonRect(rect.right() - 80, rect.top() + (rect.height() - 30) / 2, 70, 30);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#ff4444"));  // 红色背景
        painter->drawRoundedRect(buttonRect, 4, 4);  // 圆角矩形
        
        painter->setPen(Qt::white);  // 白色文字
        painter->setFont(option.font);
        painter->drawText(buttonRect, Qt::AlignCenter, "取消");
    } else {
        QString packageName = index.data(Qt::UserRole + 1).toString();
        bool isDownloaded = m_downloads.contains(packageName) && !m_downloads[packageName].isDownloading;
        
        QRect buttonRect(rect.right() - 80, rect.top() + (rect.height() - 30) / 2, 70, 30);
        painter->setPen(Qt::NoPen);
        
        if (isDownloaded) {
            // 下载完成状态
            painter->setBrush(QColor("#10B981"));  // 绿色背景
            painter->drawRoundedRect(buttonRect, 4, 4);
            painter->setPen(Qt::white);
            painter->drawText(buttonRect, Qt::AlignCenter, "下载完成");
        } else {
            // 更新按钮状态
            painter->setBrush(QColor("#e9effd"));  // 背景色
            painter->drawRoundedRect(buttonRect, 4, 4);
            painter->setPen(QColor("#2563EB"));  // 文字颜色
            painter->drawText(buttonRect, Qt::AlignCenter, "更新");
        }
    }

    painter->restore();
}

QSize AppDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(option.rect.width(), 110);
}

bool AppDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                              const QStyleOptionViewItem &option, const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QRect rect = option.rect;
        QString packageName = index.data(Qt::UserRole + 1).toString();

        if (m_downloads.contains(packageName) && m_downloads[packageName].isDownloading) {
            QRect cancelButtonRect(rect.right() - 70, rect.top() + (rect.height() - 20) / 2, 60, 20);
            if (cancelButtonRect.contains(mouseEvent->pos())) {
                m_downloadManager->cancelDownload(packageName);
                m_downloads.remove(packageName);
                emit updateDisplay(packageName);
                return true;
            }
        } else {
            QRect buttonRect(rect.right() - 80, rect.top() + (rect.height() - 30) / 2, 70, 30);
            if (buttonRect.contains(mouseEvent->pos())) {
                QString downloadUrl = index.data(Qt::UserRole + 7).toString();
                QString outputPath = QString("%1/%2.metalink").arg(QDir::tempPath(), packageName);

                m_downloads[packageName] = {0, true};
                m_downloadManager->startDownload(packageName, downloadUrl, outputPath);
                emit updateDisplay(packageName);
                return true;
            }
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
