#include "downloadmanager.h"
#include <QDebug> // 包含 QDebug 头文件

DownloadManager::DownloadManager(QObject *parent) : QObject(parent), m_process(new QProcess(this))
{
    m_progressRegex = QRegularExpression(R"(\((\d+)%\))");
    connect(m_process, &QProcess::readyReadStandardOutput, this, &DownloadManager::onProcessReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &DownloadManager::onProcessFinished);
}

DownloadManager::~DownloadManager()
{
    if (m_process->state() == QProcess::Running) {
        m_process->kill();
        m_process->waitForFinished();
    }
}

void DownloadManager::startDownload(const QString &appName)
{
    // 输出开始下载的日志
    QString program = "aptss";
    QStringList arguments;
    arguments << "download" << "--print-uris" << appName;
    
    qDebug() << "执行命令:" << program << arguments.join(" ");
    m_process->start(program, arguments);
}

void DownloadManager::onProcessReadyRead()
{
    QByteArray output = m_process->readAllStandardOutput();
    QString outputStr = QString::fromUtf8(output);

    QRegularExpressionMatch match = m_progressRegex.match(outputStr);
    if (match.hasMatch()) {
        int progress = match.captured(1).toInt();
        emit downloadProgress(progress);
    }
}

void DownloadManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    bool success = (exitStatus == QProcess::NormalExit && exitCode == 0);
    emit downloadFinished(success);
}

bool DownloadManager::isRunning() const
{
    return m_process->state() == QProcess::Running;
}

void DownloadManager::killProcess()
{
    if (m_process->state() == QProcess::Running) {
        m_process->kill();
        m_process->waitForFinished();
    }
}
