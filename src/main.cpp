#include "mainwindow.h"
#include <QApplication>
#include <QProcess>
#include <QMessageBox>
#include <unistd.h>     // for geteuid
#include <cstdlib>      // for getenv

bool isRoot() {
    return geteuid() == 0;
}

bool elevateToRoot() {
    QString program = QCoreApplication::applicationFilePath();

    // 获取当前 DISPLAY 和 XAUTHORITY 环境变量
    QByteArray display = qgetenv("DISPLAY");
    QByteArray xauthority = qgetenv("XAUTHORITY");

    // 构造带环境变量的 pkexec 命令
    QStringList args;
    args << "env"
         << "DISPLAY=" + display
         << "XAUTHORITY=" + xauthority
         << program;

    // 使用 pkexec 启动程序（带环境变量）
    return QProcess::startDetached("pkexec", args);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!isRoot()) {
        if (!elevateToRoot()) {
            QMessageBox::critical(nullptr, "权限不足", "提权失败，请确保系统支持 pkexec 并已正确安装 polkit。");
        }
        return 0;
    }

    MainWindow w;
    w.show();
    return a.exec();
}
