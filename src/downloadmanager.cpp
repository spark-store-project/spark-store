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
        // 原代码
        // m_process->waitForFinished();
        
        // 修改后增加超时处理
        if (!m_process->waitForFinished(5000)) {
            qWarning() << "进程终止超时";
        }
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
    static QString buffer;  // 缓存未处理的输出
    buffer += QString::fromUtf8(m_process->readAllStandardOutput());

    const QStringList lines = buffer.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        QRegularExpressionMatch match = m_progressRegex.match(line);
        if (match.hasMatch()) {
            int progress = match.captured(1).toInt();
            qDebug() << "匹配进度:" << progress << "% => " << line;
            emit downloadProgress(progress);
        } else {
            // qDebug() << "未匹配行:" << line;
        }
    }

    // 如果最后不是完整的一行，保留最后一部分
    if (!buffer.endsWith('\n') && !buffer.endsWith('\r')) {
        buffer = lines.last();
    } else {
        buffer.clear();
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
