#include "appdelegate.h"
#include <QIcon>
#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QProgressBar>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <QFile>

AppDelegate::AppDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_downloadManager(new DownloadManager(this)), m_installProcess(nullptr) {
    connect(m_downloadManager, &DownloadManager::downloadFinished, this,
            [this](const QString &packageName, bool success) {
        if (m_downloads.contains(packageName)) {
            m_downloads[packageName].isDownloading = false;
            // 不要提前设置 isInstalled
            emit updateDisplay(packageName);
            qDebug() << (success ? "下载完成:" : "下载失败:") << packageName;
            if (success) {
                enqueueInstall(packageName); // 安装完成后再设置 isInstalled
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
    // m_spinnerTimer.start(); // 移除这行

    // 新增：初始化和连接 QTimer
    m_spinnerUpdateTimer.setInterval(20); // 刷新间隔，可以调整
    connect(&m_spinnerUpdateTimer, &QTimer::timeout, this, &AppDelegate::updateSpinner);
}

void AppDelegate::setModel(QAbstractItemModel *model) {
    m_model = model;
}

void AppDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    // 检查是否为忽略状态
    bool isIgnored = index.data(Qt::UserRole + 8).toBool();

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    else
        painter->fillRect(option.rect, QColor("#F3F4F6"));

    // 绘制复选框
    QString packageName = index.data(Qt::UserRole + 1).toString();
    bool isSelected = m_selectedPackages.contains(packageName);
    
    QRect checkboxRect(option.rect.left() + 10, option.rect.top() + (option.rect.height() - 20) / 2, 20, 20);
    
    // 绘制复选框边框
    QColor checkboxColor = isIgnored ? QColor("#CCCCCC") : QColor("#888888");
    painter->setPen(checkboxColor);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(checkboxRect);
    
    // 如果选中，绘制勾选标记
    if (isSelected && !isIgnored) {
        painter->setPen(QPen(QColor("#2563EB"), 2));
        painter->setBrush(QColor("#2563EB"));
        painter->drawRect(checkboxRect.adjusted(4, 4, -4, -4));
    }

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

    // 调整图标位置，为复选框留出空间
    QRect iconRect(rect.left() + 40, rect.top() + (rect.height() - iconSize) / 2, iconSize, iconSize);
    
    
    // 如果是忽略状态，绘制灰色图标
    if (isIgnored) {
        // 创建灰度效果
        QPixmap originalPixmap = QIcon(iconPath).pixmap(iconSize, iconSize);
        QPixmap grayPixmap(originalPixmap.size());
        grayPixmap.fill(Qt::transparent);
        QPainter grayPainter(&grayPixmap);
        grayPainter.setOpacity(0.3); // 设置透明度使其变灰
        grayPainter.drawPixmap(0, 0, originalPixmap);
        grayPainter.end();
        painter->drawPixmap(iconRect, grayPixmap);
    } else {
        QIcon(iconPath).paint(painter, iconRect);
    }

    int textX = iconRect.right() + margin;
    int textWidth = rect.width() - textX - 100;

    QRect nameRect(textX, rect.top() + margin, textWidth, 20);
    painter->setFont(boldFont);
    QColor nameColor = isIgnored ? QColor("#999999") : QColor("#333333");
    painter->setPen(nameColor);
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, name);

    QRect versionRect(textX, nameRect.bottom() + spacing, textWidth, 20);
    painter->setFont(normalFont);
    QColor versionColor = isIgnored ? QColor("#AAAAAA") : QColor("#888888");
    painter->setPen(versionColor);
    painter->drawText(versionRect, Qt::AlignLeft | Qt::AlignVCenter,
                      QString("当前版本: %1 → 新版本: %2").arg(currentVersion, newVersion));

    QRect descRect(textX, versionRect.bottom() + spacing, textWidth, 40);
    painter->setFont(normalFont);
    QColor descColor = isIgnored ? QColor("#CCCCCC") : QColor("#AAAAAA");
    painter->setPen(descColor);
    painter->drawText(descRect, Qt::TextWordWrap,
                      QString("包大小：%1 MB").arg(QString::number(size.toDouble() / (1024 * 1024), 'f', 2)));

    bool isDownloading = m_downloads.contains(packageName) && m_downloads[packageName].isDownloading;
    int progress = m_downloads.value(packageName, DownloadInfo{0, false}).progress;
    bool isInstalled = m_downloads.value(packageName).isInstalled;
    bool isInstalling = m_downloads.value(packageName).isInstalling;

    // 如果是忽略状态，显示"已忽略"文本和"取消忽略"按钮
    if (isIgnored) {
        QRect ignoredTextRect(rect.right() - 170, rect.top() + (rect.height() - 30) / 2, 80, 30);
        painter->setPen(QColor("#999999"));
        painter->setFont(option.font);
        painter->drawText(ignoredTextRect, Qt::AlignCenter, "已忽略");
        
        // 绘制取消忽略按钮
        QRect unignoreButtonRect(rect.right() - 80, rect.top() + (rect.height() - 30) / 2, 70, 30);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#F3F4F6"));
        painter->drawRoundedRect(unignoreButtonRect, 4, 4);
        painter->setPen(QColor("#6B7280"));
        painter->drawText(unignoreButtonRect, Qt::AlignCenter, "取消忽略");
    } else if (isDownloading) {
        QRect progressRect(rect.right() - 270, rect.top() + (rect.height() - 20) / 2, 150, 20);
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = progressRect;
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = progress;
        progressBarOption.text = QString("%1%").arg(progress);
        progressBarOption.textVisible = true;
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
        
        QRect cancelButtonRect(rect.right() - 80, rect.top() + (rect.height() - 30) / 2, 70, 30);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#ff4444"));
        painter->drawRoundedRect(cancelButtonRect, 4, 4);
        
        painter->setPen(Qt::white);
        painter->setFont(option.font);
        painter->drawText(cancelButtonRect, Qt::AlignCenter, "取消");
    } else if (isInstalling) {
        QRect spinnerRect(option.rect.right() - 80, option.rect.top() + (option.rect.height() - 30) / 2, 30, 30);
        
        // 修改：使用 m_spinnerAngle
        QPen pen(QColor("#2563EB"), 3);
        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, true);
        QRectF arcRect = spinnerRect.adjusted(3, 3, -3, -3);
        painter->drawArc(arcRect, m_spinnerAngle * 16, 120 * 16); // 120度弧

        QRect textRect(option.rect.right() - 120, option.rect.top() + (option.rect.height() - 30) / 2, 110, 30);
        painter->setPen(QColor("#2563EB"));
        painter->setFont(option.font);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, "正在安装中");
    } else {
        // 绘制忽略按钮
        QRect ignoreButtonRect(option.rect.right() - 160, option.rect.top() + (option.rect.height() - 30) / 2, 70, 30);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#F3F4F6"));
        painter->drawRoundedRect(ignoreButtonRect, 4, 4);
        painter->setPen(QColor("#6B7280"));
        painter->drawText(ignoreButtonRect, Qt::AlignCenter, "忽略");

        // 绘制更新按钮
        QRect updateButtonRect(option.rect.right() - 80, option.rect.top() + (option.rect.height() - 30) / 2, 70, 30);
        painter->setPen(Qt::NoPen);
        if (isInstalled) {
            painter->setBrush(QColor("#10B981"));
            painter->drawRoundedRect(updateButtonRect, 4, 4);
            painter->setPen(Qt::white);
            painter->drawText(updateButtonRect, Qt::AlignCenter, "已安装");
        } else if (m_downloads.contains(packageName) && !m_downloads[packageName].isDownloading) {
            painter->setBrush(QColor("#10B981"));
            painter->drawRoundedRect(updateButtonRect, 4, 4);
            painter->setPen(Qt::white);
            painter->drawText(updateButtonRect, Qt::AlignCenter, "下载完成");
        } else {
            painter->setBrush(QColor("#e9effd"));
            painter->drawRoundedRect(updateButtonRect, 4, 4);
            painter->setPen(QColor("#2563EB"));
            painter->drawText(updateButtonRect, Qt::AlignCenter, "更新");
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
        
        // 检查是否为忽略状态，如果是则只允许取消忽略按钮的交互
        bool isIgnored = index.data(Qt::UserRole + 8).toBool();
        if (isIgnored) {
            QRect unignoreButtonRect(option.rect.right() - 80, option.rect.top() + (option.rect.height() - 30) / 2, 70, 30);
            if (unignoreButtonRect.contains(mouseEvent->pos())) {
                // 发送取消忽略信号
                emit unignoreApp(packageName);
                return true;
            }
            return true; // 消耗其他事件，不允许其他交互
        }

        // 检查是否点击了复选框
        QRect checkboxRect(rect.left() + 10, rect.top() + (rect.height() - 20) / 2, 20, 20);
        if (checkboxRect.contains(mouseEvent->pos())) {
            if (m_selectedPackages.contains(packageName)) {
                m_selectedPackages.remove(packageName);
            } else {
                m_selectedPackages.insert(packageName);
            }
            emit updateDisplay(packageName);
            return true;
        }

        if (m_downloads.contains(packageName) && m_downloads[packageName].isDownloading) {
            QRect cancelButtonRect(rect.right() - 70, rect.top() + (rect.height() - 20) / 2, 60, 20);
            if (cancelButtonRect.contains(mouseEvent->pos())) {
                m_downloadManager->cancelDownload(packageName);
                m_downloads.remove(packageName);
                emit updateDisplay(packageName);
                return true;
            }
        } else {
            // 检查是否点击了忽略按钮
            QRect ignoreButtonRect(rect.right() - 160, rect.top() + (rect.height() - 30) / 2, 70, 30);
            if (ignoreButtonRect.contains(mouseEvent->pos())) {
                QString currentVersion = index.data(Qt::UserRole + 2).toString();
                emit ignoreApp(packageName, currentVersion);
                return true;
            }

            // 检查是否点击了更新按钮
            QRect updateButtonRect(rect.right() - 80, rect.top() + (rect.height() - 30) / 2, 70, 30);
            if (updateButtonRect.contains(mouseEvent->pos())) {
                if (m_downloads.contains(packageName) && !m_downloads[packageName].isDownloading) {
                    return false;
                }
                
                // 检查/tmp目录下是否已经存在deb包
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
                
                // 如果存在deb包，直接进行安装
                if (!debPath.isEmpty() && QFile::exists(debPath)) {
                    qDebug() << "发现已存在的deb包，直接进行安装:" << debPath;
                    enqueueInstall(packageName);
                } else {
                    // 否则触发下载流程
                    QString downloadUrl = index.data(Qt::UserRole + 7).toString();
                    QString outputPath = QString("%1/%2.metalink").arg(QDir::tempPath(), packageName);

                    m_downloads[packageName] = {0, true};
                    m_downloadManager->startDownload(packageName, downloadUrl, outputPath);
                    emit updateDisplay(packageName);
                }
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
        if (m_downloads.contains(packageName) && (m_downloads[packageName].isDownloading || m_downloads[packageName].isInstalled))
            continue;
        QString downloadUrl = index.data(Qt::UserRole + 7).toString();
        QString outputPath = QString("%1/%2.metalink").arg(QDir::tempPath(), packageName);
        m_downloads[packageName] = {0, true, false};
        m_downloadManager->startDownload(packageName, downloadUrl, outputPath);
        emit updateDisplay(packageName);
    }
}

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
        m_spinnerUpdateTimer.stop(); // 新增：停止定时器
        return;
    }
    m_isInstalling = true;
    QString packageName = m_installQueue.dequeue();
    m_installingPackage = packageName;
    m_downloads[packageName].isInstalling = true;
    m_spinnerUpdateTimer.start(); // 新增：启动定时器
    emit updateDisplay(packageName);

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
        emit updateDisplay(packageName);
        m_installingPackage.clear();
        startNextInstall();
        return;
    }

    m_installProcess = new QProcess(this);

    QString logPath = QString("/tmp/%1_install.log").arg(packageName);
    QFile *logFile = new QFile(logPath, m_installProcess);
    if (logFile->open(QIODevice::Append | QIODevice::Text)) {
        QFile::setPermissions(logPath, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                         QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                                         QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
        connect(m_installProcess, &QProcess::readyReadStandardOutput, this, [this, packageName, logFile]() {
            QByteArray out = m_installProcess->readAllStandardOutput();
            logFile->write(out);
            logFile->flush();
            QString text = QString::fromLocal8Bit(out);
            qDebug().noquote() << text;
            if (text.contains(QStringLiteral("软件包已安装"))) {
                m_downloads[packageName].isInstalling = false;
                m_downloads[packageName].isInstalled = true;
                emit updateDisplay(packageName);
            }
        });
        connect(m_installProcess, &QProcess::readyReadStandardError, this, [this, logFile]() {
            QByteArray err = m_installProcess->readAllStandardError();
            logFile->write(err);
            logFile->flush();
            qDebug().noquote() << QString::fromLocal8Bit(err);
        });
        connect(m_installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, packageName, logFile, debPath](int exitCode, QProcess::ExitStatus status) {
            if (logFile) logFile->close();
            m_downloads[packageName].isInstalling = false;
            if (exitCode == 0) { 
                m_downloads[packageName].isInstalled = true;
                
                // 安装成功后删除deb包
                if (QFile::exists(debPath)) {
                    if (QFile::remove(debPath)) {
                        qDebug() << "已删除deb包:" << debPath;
                    } else {
                        qWarning() << "删除deb包失败:" << debPath;
                    }
                }
            }
            emit updateDisplay(packageName);
            m_installProcess->deleteLater();
            m_installProcess = nullptr;
            m_installingPackage.clear();
            startNextInstall();
        });
    } else {
        connect(m_installProcess, &QProcess::readyReadStandardOutput, this, [this, packageName, debPath]() {
            QByteArray out = m_installProcess->readAllStandardOutput();
            QString text = QString::fromLocal8Bit(out);
            qDebug().noquote() << text;
            if (text.contains(QStringLiteral("软件包已安装"))) {
                m_downloads[packageName].isInstalling = false;
                m_downloads[packageName].isInstalled = true;
                
                // 安装成功后删除deb包
                if (QFile::exists(debPath)) {
                    if (QFile::remove(debPath)) {
                        qDebug() << "已删除deb包:" << debPath;
                    } else {
                        qWarning() << "删除deb包失败:" << debPath;
                    }
                }
                
                emit updateDisplay(packageName);
            }
        });
        connect(m_installProcess, &QProcess::readyReadStandardError, this, [this]() {
            QByteArray err = m_installProcess->readAllStandardError();
            qDebug().noquote() << QString::fromLocal8Bit(err);
        });
        connect(m_installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, packageName, debPath](int exitCode, QProcess::ExitStatus /*status*/) {
            // 如果通过退出码判断安装成功，也删除deb包
            if (exitCode == 0 && QFile::exists(debPath)) {
                if (QFile::remove(debPath)) {
                    qDebug() << "已删除deb包:" << debPath;
                } else {
                    qWarning() << "删除deb包失败:" << debPath;
                }
            }
            
            emit updateDisplay(packageName);
            m_installProcess->deleteLater();
            m_installProcess = nullptr;
            m_installingPackage.clear();
            startNextInstall();
        });
    }

    QStringList args;
    args << debPath << "--no-create-desktop-entry";
    m_installProcess->start("ssinstall", args);
}

// 新增槽函数，用于更新旋转角度并触发刷新
void AppDelegate::updateSpinner() {
    m_spinnerAngle = (m_spinnerAngle + 10) % 360; // 每次增加10度
    emit updateDisplay(m_installingPackage); // 仅刷新当前正在安装的项
}

// 新增：更新选中应用的方法
void AppDelegate::startDownloadForSelected() {
    if (!m_model) return;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QModelIndex index = m_model->index(row, 0);
        QString packageName = index.data(Qt::UserRole + 1).toString();
        
        // 只下载选中的应用
        if (m_selectedPackages.contains(packageName)) {
            if (m_downloads.contains(packageName) && (m_downloads[packageName].isDownloading || m_downloads[packageName].isInstalled))
                continue;
            QString downloadUrl = index.data(Qt::UserRole + 7).toString();
            QString outputPath = QString("%1/%2.metalink").arg(QDir::tempPath(), packageName);
            m_downloads[packageName] = {0, true, false};
            m_downloadManager->startDownload(packageName, downloadUrl, outputPath);
            emit updateDisplay(packageName);
        }
    }
}

// 复选框相关方法实现
void AppDelegate::setSelectedPackages(const QSet<QString> &selected) {
    m_selectedPackages = selected;
}

QSet<QString> AppDelegate::getSelectedPackages() const {
    return m_selectedPackages;
}

void AppDelegate::clearSelection() {
    m_selectedPackages.clear();
}

// 实现获取下载状态信息的方法
const QHash<QString, DownloadInfo>& AppDelegate::getDownloads() const {
    return m_downloads;
}