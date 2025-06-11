#include "appdelegate.h"
#include <QIcon>
#include <QDebug>
#include "downloadmanager.h"
#include <QProgressBar>
#include <QPushButton>
#include <QApplication> // 包含 QApplication 头文件
#include <QDir> // 添加 QDir 头文件

AppDelegate::AppDelegate(QObject *parent) : QStyledItemDelegate(parent), m_downloadManager(new DownloadManager(this))
{
    connect(m_downloadManager, &DownloadManager::downloadProgress, this, [this](int progress) {
        m_progress = progress;
        emit updateDisplay(); // 触发重绘
    });
    connect(m_downloadManager, &DownloadManager::downloadFinished, this, [this](bool success) {
        m_isDownloading = false;
        emit updateDisplay(); // 触发重绘
        if (success) {
            qDebug() << "下载完成";
        } else {
            qDebug() << "下载失败";
        }
    });
}
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
                    QString("包大小：%1 MB").arg(QString::number(size.toDouble() / (1024 * 1024), 'f', 2)));

    if (m_isDownloading) {
        // 进度条
        QRect progressRect(rect.right() - 180, rect.top() + (rect.height() - 20) / 2, 100, 20);
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = progressRect;
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = m_progress;
        progressBarOption.text = QString("%1%").arg(m_progress);
        progressBarOption.textVisible = true;
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);

        // 取消按钮
        QRect cancelButtonRect(rect.right() - 70, rect.top() + (rect.height() - 20) / 2, 60, 20);
        QStyleOptionButton cancelButtonOption;
        cancelButtonOption.rect = cancelButtonRect;
        cancelButtonOption.text = "取消";
        cancelButtonOption.state |= QStyle::State_Enabled;
        QApplication::style()->drawControl(QStyle::CE_PushButton, &cancelButtonOption, painter);
    } else {
        // 更新按钮
        QRect buttonRect(rect.right() - 80, rect.top() + (rect.height() - 30) / 2, 70, 30);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#267AFF"));
        painter->drawRoundedRect(buttonRect, 6, 6);
        painter->setPen(Qt::white);
        painter->drawText(buttonRect, Qt::AlignCenter, "更新");
    }

    painter->restore();
}

QSize AppDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(), 110); // 每行高度 110
}

bool AppDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QRect rect = option.rect;
        if (m_isDownloading) {
            // 取消按钮区域
            QRect cancelButtonRect(rect.right() - 70, rect.top() + (rect.height() - 20) / 2, 60, 20);
            if (cancelButtonRect.contains(mouseEvent->pos())) {
                m_isDownloading = false;
                emit updateDisplay(); // 触发重绘
                return true;
            }
        } else {
            // 更新按钮区域
            QRect buttonRect(rect.right() - 80, rect.top() + (rect.height() - 30) / 2, 70, 30);
            if (buttonRect.contains(mouseEvent->pos())) {
                QString packageName = index.data(Qt::UserRole + 1).toString();
                QString downloadUrl = index.data(Qt::UserRole + 7).toString();
                QString outputPath = QString("%1/%2.metalink").arg(QDir::tempPath(), packageName);

                m_isDownloading = true;
                m_progress = 0;

                connect(m_downloadManager, &DownloadManager::downloadProgress, this, [this](int progress) {
                    m_progress = progress;
                    emit updateDisplay(); // 更新界面显示
                });

                connect(m_downloadManager, &DownloadManager::downloadFinished, this, [this](bool success) {
                    m_isDownloading = false;
                    emit updateDisplay(); // 更新界面显示
                    if (success) {
                        qDebug() << "下载完成";
                    } else {
                        qDebug() << "下载失败";
                    }
                });

                m_downloadManager->startDownload(downloadUrl, outputPath);
                emit updateDisplay(); // 触发重绘
                return true;
            }
        }
        qDebug() << "点击了第" << index.row() << "行";
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}