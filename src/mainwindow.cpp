#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    checkUpdates();
    ui->setupUi(this);
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
