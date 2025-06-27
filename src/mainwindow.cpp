#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QProcess>
#include <QMessageBox>
#include <QProgressDialog>
#include <QtConcurrent> // 新增
#include <QFutureWatcher> // 新增
#include <QIcon>
#include <qicon.h>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new AppListModel(this))
    , m_delegate(new AppDelegate(this))
{
    QIcon icon(":/resources/spark-update-tool.svg");
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
        QIcon icon(":/resources/spark-update-tool.svg");
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

        // 新增：点击“更新全部”按钮批量下载
        connect(ui->updatePushButton, &QPushButton::clicked, this, [=](){
            qDebug()<<"更新全部按钮被点击";
            m_delegate->startDownloadForAll();
        });

        checkUpdates();
        initStyle();
    });

    // 启动异步任务
    watcher->setFuture(QtConcurrent::run([this](){
        runAptssUpgrade();
    }));
}
//初始化控件样式
void MainWindow::initStyle()
{
    //设置窗口标题
    this->setWindowTitle("软件更新中心");

    //查询框样式
    ui->plainTextEdit->setStyleSheet(R"(
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
    )");

    ui->plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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
    m_model->setUpdateData(updateInfo);

    for (const auto &item : updateInfo) {
        QJsonObject obj = item.toObject();
        qDebug() << "模型设置的包名:" << obj["package"].toString();
        qDebug() << "模型设置的下载 URL:" << obj["download_url"].toString(); // 检查模型数据
    }
}

void MainWindow::runAptssUpgrade()
{
    QProcess process;
    QStringList args;
    args << "aptss" << "upgrade";
    process.start("sudo", args);
    if (!process.waitForStarted(5000)) {
        QMessageBox::warning(this, "升级失败", "无法启动 sudo aptss upgrade。");
        return;
    }
    process.write("n\n");
    process.closeWriteChannel();
    process.waitForFinished(-1);
    if (process.exitCode() != 0) {
        QMessageBox::warning(this, "升级失败", "执行 sudo aptss upgrade 失败，请检查系统环境。");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
