#include "mainwindow.h"
#include <QApplication>
#include <QProcess>
#include <QMessageBox>
#include <unistd.h>     // for geteuid
#include <cstdlib>      // for getenv
#include <QDebug>       // For debugging output

bool isRoot() {
    return geteuid() == 0;
}

bool elevateToRoot() {
    QString program = QCoreApplication::applicationFilePath();
    qDebug() << "Current application path:" << program;

    QByteArray display = qgetenv("DISPLAY");
    QByteArray xauthority = qgetenv("XAUTHORITY");
    QByteArray home = qgetenv("HOME");  // 获取原始用户的 HOME 目境变量

    QStringList args;
    args << "env"
         << "DISPLAY=" + display
         << "XAUTHORITY=" + xauthority
         << "SUDO_USER_HOME=" + home  // 传递原始用户的 HOME 路径
         << program;

    QProcess process;
    process.setProgram("pkexec");
    process.setArguments(args);

    qDebug() << "Attempting to elevate using pkexec with arguments:" << args;

    process.start();
    if (!process.waitForStarted(5000)) {
        qDebug() << "Failed to start pkexec.";
        return false;
    }

    // 阻塞等待提权进程退出（比如主程序窗口关闭）
    if (!process.waitForFinished(-1)) {  // 等待直到新进程退出
        qDebug() << "pkexec process waitForFinished failed.";
        return false;
    }

    int exitCode = process.exitCode();
    QProcess::ExitStatus exitStatus = process.exitStatus();

    qDebug() << "pkexec exit code:" << exitCode;
    qDebug() << "pkexec exit status:" << exitStatus;
    qDebug() << "pkexec stderr:" << process.readAllStandardError();

    return (exitStatus == QProcess::NormalExit && exitCode == 0);
}

int main(int argc, char *argv[])
{
    // 必须在 QGuiApplication 实例创建之前调用
    // QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/resources/128*128/spark-update-tool.png"));
    if (!isRoot()) {
        qDebug() << "Not running as root. Attempting to elevate...";
        if (!elevateToRoot()) {
            qDebug() << "Elevation failed or pkexec command was not executed successfully.";
            QMessageBox::critical(nullptr,
                                  "权限不足",
                                  "提权失败！\n\n您的系统可能不支持 `pkexec` 或 `polkit` 配置不正确，"
                                  "或者您取消了授权。\n\n请尝试使用 `sudo` 命令运行此程序："
                                  "\n\n在终端中输入：\n`sudo " + QCoreApplication::applicationName() + "`");
            return 0; // 提权失败，退出程序
        } else {
            // 如果 elevateToRoot 返回 true，说明 pkexec 命令本身执行成功
            // 但这并不意味着原始程序以 root 权限启动了
            // 因为 elevateToRoot 启动的是一个新的进程，当前进程应该退出
            // 否则会并行运行两个程序实例
            qDebug() << "pkexec command executed successfully (new process likely started). Exiting current process.";
            return 0; // 当前非root进程退出
        }
    } else {
        qDebug() << "Running as root.";
    }

    MainWindow w;
    w.show();
    return a.exec();
}