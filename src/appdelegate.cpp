#include "appdelegate.h"
#include <QIcon>
#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QProgressBar>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <QProcess> // 确保 QProcess 头文件已包含
#include <QFile>    // 确保 QFile 头文件已包含
#include <QQueue>   // 确保 QQueue 头文件已包含

AppDelegate::AppDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_downloadManager(new DownloadManager(this)), m_installProcess(nullptr) {
    connect(m_downloadManager, &DownloadManager::downloadFinished, this,
            [this](const QString &packageName, bool success) {
        if (m_downloads.contains(packageName)) {
            m_downloads[packageName].isDownloading = false;
            emit updateDisplay(packageName);
            qDebug() << (success ? "下载完成:" : "下载失败:") << packageName;
            if (success) {
                enqueueInstall(packageName);
            }
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
    m_spinnerTimer.start();
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
    // QString description = index.data(Qt::UserRole + 6).toString(); // description 未使用

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
    int progress = m_downloads.value(packageName, DownloadInfo{0, false, false, false}).progress; // 确保 DownloadInfo 有默认构造或匹配的初始化
    bool isInstalled = m_downloads.value(packageName).isInstalled;
    bool isInstalling = m_downloads.value(packageName).isInstalling;

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
    } else if (isInstalling) {
        // 安装中：显示转圈和文字
        QRect spinnerRect(option.rect.right() - 80, option.rect.top() + (option.rect.height() - 30) / 2, 30, 30);
        int angle = (m_spinnerTimer.elapsed() / 10) % 360;
        QPen pen(QColor("#2563EB"), 3);
        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, true);
        QRectF arcRect = spinnerRect.adjusted(3, 3, -3, -3);
        painter->drawArc(arcRect, angle * 16, 120 * 16); // 120度弧

        QRect textRect(option.rect.right() - 120, option.rect.top() + (option.rect.height() - 30) / 2, 110, 30);
        painter->setPen(QColor("#2563EB"));
        painter->setFont(option.font);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, "正在安装中");
    } else {
        QRect buttonRect(option.rect.right() - 80, option.rect.top() + (option.rect.height() - 30) / 2, 70, 30);
        painter->setPen(Qt::NoPen);
        if (isInstalled) {
            painter->setBrush(QColor("#10B981"));
            painter->drawRoundedRect(buttonRect, 4, 4);
            painter->setPen(Qt::white);
            painter->drawText(buttonRect, Qt::AlignCenter, "已安装");
        } else if (m_downloads.contains(packageName) && !m_downloads[packageName].isDownloading) {
            // 下载完成，按钮绿色，样式不变
            painter->setBrush(QColor("#10B981"));
            painter->drawRoundedRect(buttonRect, 4, 4);
            painter->setPen(Qt::white);
            painter->drawText(buttonRect, Qt::AlignCenter, "下载完成");
            // 不需要特殊处理样式，交互在 editorEvent 控制
        } else {
            painter->setBrush(QColor("#e9effd"));
            painter->drawRoundedRect(buttonRect, 4, 4);
            painter->setPen(QColor("#2563EB"));
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
                // 判断是否为“下载完成”状态，如果是则不响应点击
                // 同时也要判断是否为“已安装”状态，如果是也不响应点击
                if ((m_downloads.contains(packageName) && !m_downloads[packageName].isDownloading) ||
                    (m_downloads.contains(packageName) && m_downloads[packageName].isInstalled)) {
                    // “下载完成”或“已安装”状态，按钮失效，点击无效
                    return false;
                }
                QString downloadUrl = index.data(Qt::UserRole + 7).toString();
                QString outputPath = QString("%1/%2.metalink").arg(QDir::tempPath(), packageName);

                // 假设 DownloadInfo 结构体的构造函数或聚合初始化方式支持四个成员
                m_downloads[packageName] = {0, true, false, false}; // progress, isDownloading, isInstalling, isInstalled
                m_downloadManager->startDownload(packageName, downloadUrl, outputPath);
                emit updateDisplay(packageName);
                return true;
            }
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void AppDelegate::startDownloadForAll() {
    if (!m_model) return;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QModelIndex index = m_model->index(row, 0);
        QString packageName = index.data(Qt::UserRole + 1).toString();
        // 确保跳过正在下载、正在安装或已安装的
        if (m_downloads.contains(packageName) &&
            (m_downloads[packageName].isDownloading ||
             m_downloads[packageName].isInstalling ||
             m_downloads[packageName].isInstalled)) {
            continue;
        }
        QString downloadUrl = index.data(Qt::UserRole + 7).toString();
        QString outputPath = QString("%1/%2.metalink").arg(QDir::tempPath(), packageName);
        m_downloads[packageName] = {0, true, false, false}; // progress, isDownloading, isInstalling, isInstalled
        m_downloadManager->startDownload(packageName, downloadUrl, outputPath);
        emit updateDisplay(packageName);
    }
}

// 新增：安装队列相关实现
void AppDelegate::enqueueInstall(const QString &packageName) {
    m_installQueue.enqueue(packageName);
    if (!m_isInstalling) {
        startNextInstall();
    }
}

void AppDelegate::startNextInstall() {
    if (m_installQueue.isEmpty()) {
        m_isInstalling = false;
        m_installingPackage.clear();
        return;
    }
    m_isInstalling = true;
    QString packageName = m_installQueue.dequeue();
    m_installingPackage = packageName;
    m_downloads[packageName].isInstalling = true;
    emit updateDisplay(packageName);

    // 查找 /tmp 下以包名开头的 .deb 文件
    QDir tempDir(QDir::tempPath());
    QStringList debs = tempDir.entryList(QStringList() << QString("%1_*.deb").arg(packageName), QDir::Files);
    QString debPath;
    if (!debs.isEmpty()) {
        debPath = tempDir.absoluteFilePath(debs.first());
    } else {
        debs = tempDir.entryList(QStringList() << QString("%1*.deb").arg(packageName), QDir::Files);
        if (!debs.isEmpty()) {
            debPath = tempDir.absoluteFilePath(debs.first());
        }
    }

    if (debPath.isEmpty()) {
        qWarning() << "未找到deb文件，包名:" << packageName;
        m_downloads[packageName].isInstalling = false;
        // 如果找不到deb文件，也应该根据情况处理是已安装还是未安装
        // 这里只是简单地设置为未安装，并更新显示
        m_downloads[packageName].isInstalled = false; // 确保如果未找到文件则不显示已安装
        emit updateDisplay(packageName);
        m_installingPackage.clear();
        startNextInstall();
        return;
    }

    m_installProcess = new QProcess(this);

    // 新增：准备安装日志文件
    QString logPath = QString("/tmp/%1_install.log").arg(packageName);
    QFile *logFile = new QFile(logPath, m_installProcess);
    if (logFile->open(QIODevice::Append | QIODevice::Text)) {
        // 设置权限为777
        QFile::setPermissions(logPath, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                         QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                                         QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
        connect(m_installProcess, &QProcess::readyReadStandardOutput, this, [this, packageName, logFile]() {
            QByteArray out = m_installProcess->readAllStandardOutput();
            logFile->write(out);
            logFile->flush();
            QString text = QString::fromLocal8Bit(out);
            qDebug().noquote() << text;
            // 尽管此处可以检查关键字，但最终的安装状态应由进程退出码决定
        });
        connect(m_installProcess, &QProcess::readyReadStandardError, this, [this, logFile]() {
            QByteArray err = m_installProcess->readAllStandardError();
            logFile->write(err);
            logFile->flush();
            qDebug().noquote() << QString::fromLocal8Bit(err);
        });
        connect(m_installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, packageName, logFile](int exitCode, QProcess::ExitStatus exitStatus) {
            if (logFile) logFile->close();

            // === 关键修改 ===
            // 检查进程是否正常退出且退出码为0，这通常表示成功
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                m_downloads[packageName].isInstalled = true; // 安装成功，设置为已安装
                qDebug() << "安装成功:" << packageName;
            } else {
                // 安装失败或异常退出
                m_downloads[packageName].isInstalled = false; // 确保不是已安装状态
                qWarning() << "安装失败或异常退出:" << packageName << "Exit Code:" << exitCode << "Exit Status:" << exitStatus;
            }
            // === 关键修改结束 ===

            m_downloads[packageName].isInstalling = false; // 安装过程结束
            emit updateDisplay(packageName); // 更新UI显示

            m_installProcess->deleteLater();
            m_installProcess = nullptr;
            m_installingPackage.clear();
            startNextInstall(); // 处理队列中的下一个安装任务
        });
    } else {
        // 日志文件无法打开时，仍然要连接原有信号
        connect(m_installProcess, &QProcess::readyReadStandardOutput, this, [this, packageName]() {
            QByteArray out = m_installProcess->readAllStandardOutput();
            QString text = QString::fromLocal8Bit(out);
            qDebug().noquote() << text;
            // 同样，这里的关键字检查不是最可靠的
        });
        connect(m_installProcess, &QProcess::readyReadStandardError, this, [this]() {
            QByteArray err = m_installProcess->readAllStandardError();
            qDebug().noquote() << QString::fromLocal8Bit(err);
        });
        connect(m_installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, packageName](int exitCode, QProcess::ExitStatus exitStatus) {
            // === 关键修改 (与上面相同) ===
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                m_downloads[packageName].isInstalled = true;
                qDebug() << "安装成功 (无日志文件):" << packageName;
            } else {
                m_downloads[packageName].isInstalled = false;
                qWarning() << "安装失败或异常退出 (无日志文件):" << packageName << "Exit Code:" << exitCode << "Exit Status:" << exitStatus;
            }
            // === 关键修改结束 ===

            m_downloads[packageName].isInstalling = false;
            emit updateDisplay(packageName);
            m_installProcess->deleteLater();
            m_installProcess = nullptr;
            m_installingPackage.clear();
            startNextInstall();
        });
    }

    // 注意参数顺序：deb路径在前，--no-create-desktop-entry在后
    QStringList args;
    args << debPath << "--no-create-desktop-entry";
    m_installProcess->start("ssinstall", args);
}

