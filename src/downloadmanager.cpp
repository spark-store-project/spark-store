#include "downloadmanager.h"

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
    QString command = QString("aptss download --print-uris %1").arg(appName);
    m_process->start(command);
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
