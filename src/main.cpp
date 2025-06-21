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

    // 获取当前 DISPLAY 和 XAUTHORITY 环境变量
    QByteArray display = qgetenv("DISPLAY");
    QByteArray xauthority = qgetenv("XAUTHORITY");
    qDebug() << "DISPLAY:" << display;
    qDebug() << "XAUTHORITY:" << xauthority;

    // 构造带环境变量的 pkexec 命令
    QStringList args;
    args << "env"
         << "DISPLAY=" + display
         << "XAUTHORITY=" + xauthority
         << program;

    QProcess *process = new QProcess();
    process->setProgram("pkexec");
    process->setArguments(args);

    qDebug() << "Attempting to elevate using pkexec with arguments:" << args;

    // 启动进程并等待完成
    process->start();
    // 等待 pkexec 完成，通常会弹出密码框
    // 设置一个合理的超时时间，例如 30 秒
    if (!process->waitForFinished(30000)) {
        qDebug() << "pkexec process timed out.";
        process->kill(); // 如果超时，终止进程
        delete process;
        return false;
    }

    int exitCode = process->exitCode();
    QProcess::ExitStatus exitStatus = process->exitStatus();

    qDebug() << "pkexec exit code:" << exitCode;
    qDebug() << "pkexec exit status:" << exitStatus;
    qDebug() << "pkexec standard error:" << process->readAllStandardError();

    // 检查 pkexec 是否成功
    // pkexec 成功执行通常返回 0，或者如果用户取消授权，会返回非零值。
    // 但是，如果 pkexec 本身执行失败（例如找不到命令），exitStatus 会是 CrashOrKilled
    // 这里我们认为只要pkexec成功启动了，即使授权失败，也算pkexec命令执行成功了，
    // 因为这表示pkexec本身是可用的。
    // 只有当pkexec命令无法执行时，才认为是pkexec的问题
    bool pkexecCommandExecuted = (exitStatus == QProcess::NormalExit && exitCode == 0);

    delete process;
    return pkexecCommandExecuted;
}

int main(int argc, char *argv[])
{
    // 必须在 QGuiApplication 实例创建之前调用
    // QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication a(argc, argv);

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