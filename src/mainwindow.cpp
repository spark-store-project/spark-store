#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new AppListModel(this))
    , m_delegate(new AppDelegate(this))
{
    ui->setupUi(this);

    // 创建 QListView 并设置父控件为 ui->appWidget
    listView = new QListView(ui->appWidget);
    listView->setModel(m_model);
    listView->setItemDelegate(m_delegate);

    // 设置 QListView 填充 ui->appWidget
    QVBoxLayout *layout = new QVBoxLayout(ui->appWidget);
    layout->addWidget(listView);
    layout->setContentsMargins(0, 0, 0, 0);

    checkUpdates();
    initStyle();
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
    listView->setStyleSheet(R"(
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

MainWindow::~MainWindow()
{
    delete ui;
}
