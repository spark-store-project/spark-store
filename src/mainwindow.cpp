#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    checkUpdates();
    ui->setupUi(this);
    initStyle();
}
//初始化控件样式
void MainWindow::initStyle()
{
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


}
void MainWindow::checkUpdates()
{
    aptssUpdater updater;

    // 获取可更新包列表
    QStringList updateablePackages = updater.getUpdateablePackages();
    QStringList packageSizes = updater.getPackageSizes();
    QStringList appNames = updater.getDesktopAppNames();
    qDebug() << "可更新包列表:";
    for (int i = 0; i < updateablePackages.size(); ++i) {
        QString packageInfo = updateablePackages.at(i);
        QString packageSize = (i < packageSizes.size()) ? packageSizes.at(i) : "未知大小";
        qDebug() << QString("%1 (%2)").arg(packageInfo, packageSize);
    }
    for (const QString &appName : appNames) {
        qDebug() << appName;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
