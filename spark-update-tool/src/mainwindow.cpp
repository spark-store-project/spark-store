#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QProcess>
#include <QMessageBox>
#include <QProgressDialog>
#include <QtConcurrent> // 新增
#include <QFutureWatcher> // 新增
#include <QIcon>
#include <qicon.h>
#include <unistd.h>     // for geteuid
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new AppListModel(this))
    , m_delegate(new AppDelegate(this))
    , m_ignoreConfig(new IgnoreConfig(this))
{
    QIcon icon(":/resources/128*128/spark-update-tool.png");
    setWindowIcon(icon);
    QProgressDialog *progressDialog = new QProgressDialog("正在与服务器通信，获取更新信息中...", QString(), 0, 0, this);
    progressDialog->setWindowModality(Qt::ApplicationModal);
    progressDialog->setCancelButton(nullptr);
    progressDialog->setWindowTitle("请稍候");
    progressDialog->setMinimumDuration(0);
    progressDialog->setWindowFlags(progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint); // 禁用关闭按钮
    progressDialog->show();
    //异步执行runAptssUpgrade
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
        progressDialog->close();
        progressDialog->deleteLater();
        watcher->deleteLater();
        ui->setupUi(this);
        QIcon icon(":/resources/128*128/spark-update-tool.png");
        setWindowIcon(icon);
        // 创建 QListView 并设置父控件为 ui->appWidget
        listView = new QListView(ui->appWidget);
        listView->setModel(m_model);
        listView->setItemDelegate(m_delegate);

        // 新增：确保 delegate 拥有 model 指针
        m_delegate->setModel(m_model);

        // 设置 QListView 填充 ui->appWidget
        QVBoxLayout *layout = new QVBoxLayout(ui->appWidget);
        layout->addWidget(listView);
        layout->setContentsMargins(0, 0, 0, 0);
        connect(m_delegate, &AppDelegate::updateDisplay, this, [=](const QString &packageName) {
            for (int i = 0; i < m_model->rowCount(); ++i) {
                QModelIndex index = m_model->index(i);
                if (index.data(Qt::UserRole + 1).toString() == packageName) {
                    m_model->dataChanged(index, index); // 刷新该行
                    break;
                }
            }
        });

        // 连接应用委托的信号
    connect(m_delegate, &AppDelegate::ignoreApp, this, &MainWindow::onIgnoreApp);
    connect(m_delegate, &AppDelegate::unignoreApp, this, &MainWindow::onUnignoreApp);

        // 新增：点击“更新全部”按钮批量下载
        connect(ui->updatePushButton, &QPushButton::clicked, this, [=](){
            qDebug()<<"更新按钮被点击";
            if (m_delegate->getSelectedPackages().isEmpty()) {
                // 没有选中任何应用，更新全部
                m_delegate->startDownloadForAll();
            } else {
                // 有选中应用，更新选中
                m_delegate->startDownloadForSelected();
                m_delegate->clearSelection();
                updateButtonText();
            }
        });
        
        // 新增：监听选择变化
        connect(m_delegate, &AppDelegate::updateDisplay, this, &MainWindow::handleSelectionChanged);

        checkUpdates();
        // 新增：监听搜索框文本变化
        connect(ui->searchPlainTextEdit, &QPlainTextEdit::textChanged, this, [=]() {
            QString keyword = ui->searchPlainTextEdit->toPlainText();
            filterAppsByKeyword(keyword);
        });
        initStyle();

        // 确保搜索框内容为空，placeholder 能显示
        ui->searchPlainTextEdit->clear();
    });

    // 启动异步任务
    watcher->setFuture(QtConcurrent::run([this](){
        runAptssUpgrade();
    }));
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = screenGeometry.x() + (screenGeometry.width() - this->width()) / 2;
    int y = screenGeometry.y() + (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);
}
//初始化控件样式
void MainWindow::initStyle()
{
    //设置窗口标题
    this->setWindowTitle("软件更新中心");

    //查询框样式
    ui->searchPlainTextEdit->setStyleSheet(R"(
        QPlainTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            border-radius: 4px;
            padding-top: 8px;
            padding-bottom: 8px;
            font-size: 9px;
            line-height: 1.4;
            color: #9CA3AF;
        }
        QPlainTextEdit[placeholderText]:empty {
            color: #9CA3AF;
        }
    )");

    ui->searchPlainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->searchPlainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //筛选框样式
    ui->FilterComboBox->setStyleSheet(R"(
        QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            border-radius: 4px;
            color: #4B5563;
            padding: 4px 8px;
        }

        QComboBox::drop-down {
            border: none;
            width: 20px;
        }

        QComboBox::down-arrow {
            image: url(:/resources/down_arrow.svg);
            width: 12px;
            height: 16px;
        }
        QComboBox QAbstractItemView {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            color: #4B5563;
            selection-background-color: #F3F4F6;
            selection-color: #111827;
        }
    )");

    //更新软件按钮样式
    ui->updatePushButton->setStyleSheet(R"(
        QPushButton {
            background-color: #2563EB;
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            font-size: 14px;
            padding: 6px 12px;
            text-align: center;
        }

        QPushButton:hover {
            background-color: #1D4ED8; /* 深一点的 hover 效果，可选 */
        }

        QPushButton:pressed {
            background-color: #1E40AF; /* 按下效果，可选 */
        }

        QPushButton:disabled {
            background-color: #A5B4FC;
            color: #F9FAFB;
        }
    )");

    //设置背景填充颜色
    ui->backgroundWidget->setStyleSheet(R"(
        QWidget {
            background-color: #FFFFFF;
            border-radius: 12px;
        }
    )");

    //设置主背景颜色
    this->setStyleSheet("background-color: #F8FAFC;");

    // 添加滚动条样式
    this->setStyleSheet(R"(
        QScrollBar:vertical {
            background: #F3F4F6;
            width: 8px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #D1D5DB;
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #9CA3AF;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            background: none;
            height: 0px;
        }
        
        QScrollBar:horizontal {
            background: #F3F4F6;
            height: 8px;
            margin: 0px;
        }
        QScrollBar::handle:horizontal {
            background: #D1D5DB;
            border-radius: 4px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #9CA3AF;
        }
    )");
}
void MainWindow::checkUpdates()
{
    aptssUpdater updater;
    QJsonArray updateInfo = updater.getUpdateInfoAsJson();
    
    // 分离正常应用和忽略应用
    QJsonArray normalApps;
    QJsonArray ignoredApps;
    
    for (const auto &item : updateInfo) {
        QJsonObject obj = item.toObject();
        QString packageName = obj["package"].toString();
        QString currentVersion = obj["current_version"].toString();
        
        // 检查应用是否被忽略
        if (m_ignoreConfig->isAppIgnored(packageName, currentVersion)) {
            // 标记为忽略状态
            obj["ignored"] = true;
            ignoredApps.append(obj);
        } else {
            obj["ignored"] = false;
            normalApps.append(obj);
        }
    }
    
    // 合并数组：正常应用在前，忽略应用在后
    QJsonArray finalApps;
    for (const auto &item : normalApps) {
        finalApps.append(item);
    }
    for (const auto &item : ignoredApps) {
        finalApps.append(item);
    }
    
    m_allApps = finalApps; // 保存所有应用数据
    m_model->setUpdateData(finalApps);

    for (const auto &item : finalApps) {
        QJsonObject obj = item.toObject();
        qDebug() << "模型设置的包名:" << obj["package"].toString() << "忽略状态:" << obj["ignored"].toBool();
        qDebug() << "模型设置的下载 URL:" << obj["download_url"].toString(); // 检查模型数据
    }
}

// 新增：根据关键字过滤应用
void MainWindow::filterAppsByKeyword(const QString &keyword)
{
    if (keyword.trimmed().isEmpty()) {
        m_model->setUpdateData(m_allApps);
        return;
    }
    
    // 分离正常应用和忽略应用
    QJsonArray normalApps;
    QJsonArray ignoredApps;
    
    for (const auto &item : m_allApps) {
        QJsonObject obj = item.toObject();
        // 可根据需要匹配更多字段
        QString name = obj.value("name").toString();
        QString package = obj.value("package").toString();
        if (name.contains(keyword, Qt::CaseInsensitive) ||
            package.contains(keyword, Qt::CaseInsensitive)) {
            
            // 检查是否为忽略状态
            if (obj.value("ignored").toBool()) {
                ignoredApps.append(item);
            } else {
                normalApps.append(item);
            }
        }
    }
    
    // 合并数组：正常应用在前，忽略应用在后
    QJsonArray filtered;
    for (const auto &item : normalApps) {
        filtered.append(item);
    }
    for (const auto &item : ignoredApps) {
        filtered.append(item);
    }
    
    m_model->setUpdateData(filtered);
}

void MainWindow::runAptssUpgrade()
{
    QProcess process;
    
    // 检查是否已经是root用户，如果是则直接执行命令，否则使用sudo
    if (geteuid() == 0) {
        // root用户直接执行
        process.start("aptss", QStringList() << "ssupdate");
    } else {
        // 非root用户使用sudo
        process.start("sudo", QStringList() << "aptss" << "ssupdate");
    }
    
    if (!process.waitForStarted(5000)) {
        QMessageBox::warning(this, "升级失败", "无法启动 aptss ssupdate");
        return;
    }
    process.write("n\n");
    process.closeWriteChannel();
    
    // 设置超时时间，避免无限等待
    if (!process.waitForFinished(30000)) { // 30秒超时
        qDebug() << "aptss ssupdate 执行超时";
        process.kill(); // 强制终止进程
        return;
    }
    
    if (process.exitCode() != 0) {
        QMessageBox::warning(this, "升级失败", "执行 aptss ssupdate 失败，请检查系统环境或稍后再试。");
    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    // 检查是否正在进行更新
    bool isUpdating = false;
    
    // 通过AppDelegate检查是否有正在下载或安装的应用
    const QHash<QString, DownloadInfo>& downloads = m_delegate->getDownloads();
    for (auto it = downloads.constBegin(); it != downloads.constEnd(); ++it) {
        if (it.value().isDownloading || it.value().isInstalling) {
            isUpdating = true;
            break;
        }
    }
    
    // 如果正在更新，才显示确认对话框
    if (isUpdating) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "确认关闭", "正在更新，是否确认关闭窗口？", QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        // 如果没有更新，直接关闭窗口
        event->accept();
    }
}
void MainWindow::handleUpdateFinished(bool success)
{
    if (success) {
        // 更新成功时的处理逻辑
        QMessageBox::information(this, "更新完成", "软件更新已成功完成！");
    } else {
        // 更新失败时的处理逻辑
        QMessageBox::warning(this, "更新失败", "软件更新过程中出现错误，请稍后再试。");
    }
    
    // 刷新应用列表
    checkUpdates();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 新增：更新按钮文本
void MainWindow::updateButtonText() {
    int selectedCount = m_delegate->getSelectedPackages().size();
    if (selectedCount > 0) {
        ui->updatePushButton->setText(QString("更新选中(%1)").arg(selectedCount));
    } else {
        ui->updatePushButton->setText("更新全部");
    }
}

// 新增：处理选择变化
void MainWindow::handleSelectionChanged() {
    updateButtonText();
}

// 新增：处理忽略应用的槽函数
void MainWindow::onIgnoreApp(const QString &packageName, const QString &version) {
    // 将应用添加到忽略配置中
    m_ignoreConfig->addIgnoredApp(packageName, version);
    
    // 更新模型中应用的状态，而不是移除
    QJsonArray updatedApps;
    for (const auto &item : m_allApps) {
        QJsonObject obj = item.toObject();
        if (obj["package"].toString() == packageName) {
            obj["ignored"] = true; // 标记为忽略状态
        }
        updatedApps.append(obj);
    }
    m_allApps = updatedApps;
    
    // 重新排序：正常应用在前，忽略应用在后
    checkUpdates();
}

// 新增：处理取消忽略应用的槽函数
void MainWindow::onUnignoreApp(const QString &packageName) {
    // 从忽略配置中移除应用
    m_ignoreConfig->removeIgnoredApp(packageName);
    
    // 更新模型中应用的状态
    QJsonArray updatedApps;
    for (const auto &item : m_allApps) {
        QJsonObject obj = item.toObject();
        if (obj["package"].toString() == packageName) {
            obj["ignored"] = false; // 标记为非忽略状态
        }
        updatedApps.append(obj);
    }
    m_allApps = updatedApps;
    
    // 重新排序：正常应用在前，忽略应用在后
    checkUpdates();
}