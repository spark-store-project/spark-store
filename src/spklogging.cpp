
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include "spklogging.h"
#include "spkmsgbox.h"
#include "spkuimsg.h"
#include "spkutils.h"

SpkLogger *SpkLogger::Instance = nullptr;

SpkLogger::SpkLogger(QString suggestPath)
{
  Initialize(suggestPath);

  // Initialize libnotify
  if(!notify_init(QObject::tr("Spark\\ Store").toLocal8Bit()))
  {
    sErrPop(QObject::tr("libnotify failed to initialize. "
                        "Desktop notifications will not work."));
  }
  else
    NotifyViable = true;
}

SpkLogger::~SpkLogger()
{
  if(NotifyViable)
    notify_uninit();
}

void SpkLogger::Initialize(QString suggestPath)
{
  if(Instance != nullptr) // Single instance
    return;

  if(suggestPath != "")
  {
    if(QFile::permissions(suggestPath).testFlag(QFileDevice::WriteUser))
      mLogPath = suggestPath;
  }
  mLogPath = QDir::homePath() + "/.local/share/spark-store/logs/default.log";
  QString path = mLogPath.section('/', 1, -2, QString::SectionIncludeLeadingSep);
  if(!SpkUtils::EnsureDirExists(path))
  {
    SpkLogger::Error(QObject::tr("Log directory \"%1\" cannot be created.").arg(path));
    return;
  }

  mLogFile.setFileName(mLogPath);
  mLogFile.open(QFile::WriteOnly | QFile::Append);
  if(!mLogFile.isWritable())
  {
    Error(QObject::tr("Log file \"%1\" is not a writable file. "
                      "Please check the permissions, or if it's a directory.").arg(mLogPath), true);
    return;
  }
  mLogWriter.setDevice(&mLogFile);

  Instance = this;
}

void SpkLogger::Log(QString message)
{
  message = QDateTime::currentDateTime().toString() + "[Log]" + message;
  mLogData.insertRow(mLogData.rowCount());
  auto id = mLogData.index(mLogData.rowCount() - 1, 0);
  mLogData.setData(id, message);
  mLogWriter << message << "\n";
}

void SpkLogger::Warning(QString message)
{
  message = QDateTime::currentDateTime().toString() + "[Warning]" + message;
  mLogData.insertRow(mLogData.rowCount());
  auto id = mLogData.index(mLogData.rowCount() - 1, 0);
  mLogData.setData(id, message);
  mLogWriter << message << "\n";
}

void SpkLogger::Error(QString message, const bool pop)
{
  message = QDateTime::currentDateTime().toString() + "[Error]" + message;
  mLogData.insertRow(mLogData.rowCount());
  auto id = mLogData.index(mLogData.rowCount() - 1, 0);
  mLogData.setData(id, message);
  mLogWriter << message << "\n";
  if(pop)
  {
//    QMessageBox msgbox;
//    msgbox.setIcon(QMessageBox::Critical);
//    msgbox.setText(QObject::tr("Spark Store has encountered an error.\n"
//                               "Parts of the experience is expected to be broken.\n\n"
//                               "Details:\n%1")
//                               .arg(message));
//    msgbox.exec(); // I don't know whether we need to show it non-modal.
    SpkMsgBox::StaticExec(QObject::tr("Spark Store has encountered an error.\n"
                               "Parts of the experience is expected to be broken.\n\n"),
                          QObject::tr("Spark Store Error"),
                          QMessageBox::Critical,
                          QMessageBox::Ok,
                          message,
                          true);
  }
}

void SpkLogger::Critical(QString message)
{
  message = QDateTime::currentDateTime().toString() + "[Critical]" + message;
  mLogData.insertRow(mLogData.rowCount());
  auto id = mLogData.index(mLogData.rowCount() - 1, 0);
  mLogData.setData(id, message);
  mLogWriter << message << "\n";
  QMessageBox::critical(nullptr,
                        QObject::tr("Critical error in Spark Store"),
                        QObject::tr("Spark Store has encountered a fatal error and cannot"
                                    "continue anymore. Please consider checking logs and send a"
                                    "bug report to us.\n\n"
                                    "Details:\n%1")
                                    .arg(message));
  exit(2);
}

void SpkLogger::Notify(QString message)
{
  SpkUiMessage::SendStoreNotification(message);
}

