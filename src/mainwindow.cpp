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
    qDebug() << "可更新包列表:";
    for (const QString &package : updateablePackages) {
        qDebug() << package;
    }


}

MainWindow::~MainWindow()
{
    delete ui;
}
