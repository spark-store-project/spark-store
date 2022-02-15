
#include <QFile>
#include "pkgs/spkpkgmgrapt.h"

SpkPkgMgrApt::SpkPkgMgrApt(QObject *parent) :
  SpkPkgMgrBase(parent)
{
  mActDesc = new QAction("Debian APT", this);
  mActDesc->setDisabled(true);

  mActAptTerm = new QAction(tr("APT (in terminal)"), this);
  mActAptitudeTerm = new QAction(tr("Aptitude (in terminal)"), this);
  mActGdebi = new QAction(tr("GDebi"), this);
  mActDeepinPkgInst = new QAction(tr("Deepin Package Installer"), this);

  mMenu->insertAction(mActOpen, mActAptTerm);
  mMenu->insertAction(mActOpen, mActAptitudeTerm);
  mMenu->insertAction(mActOpen, mActGdebi);
  mMenu->insertAction(mActOpen, mActDeepinPkgInst);
  mMenu->insertSeparator(mActOpen);
  mMenu->addSeparator();
  mMenu->addAction(mActDesc);

  connect(&mInstaller, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this, &SpkPkgMgrApt::InstallerExited);
}

bool SpkPkgMgrApt::DetectRequirements()
{
  return QFile::exists("/usr/bin/apt") &&
      QFile::exists("/usr/bin/dpkg");
}

SpkPkgMgrBase::PkgInstallResult
SpkPkgMgrApt::ExecuteInstallation(QString pkgPath, int entryId)
{
  static auto prompt = tr("\n\n================================\n"
                              "Press Enter to continue.");

  if(mInstaller.state() == QProcess::Running)
  {
    sNotify(tr("Please wait till the current installation finishes."));
    return Ignored;
  }
  mCurrentItemId = entryId;
  CheckInstallerAvailability();
  auto item = mMenu->exec(QCursor::pos());
  if(item == mActOpenDir)
  {
    QDesktopServices::openUrl(QUrl(SpkUtils::CutPath(pkgPath)));
    return Ignored;
  }
  else if(item == mActOpen)
  {
    QDesktopServices::openUrl(QUrl(pkgPath));
    return Ignored;
  }
  else if(item == mActAptTerm)
  {
    mInstaller.setProgram(SpkUtils::AvailableTerminal.first);
    mInstaller.setArguments({ SpkUtils::AvailableTerminal.second, "pkexec",
                              "bash", "-c",
                              QString("apt install '%1'; R=$?; read -p '%2'; exit $R")
                                .arg(pkgPath, prompt)});
    mInstaller.start();
  }
  else if(item == mActAptitudeTerm)
  {
    mInstaller.setProgram(SpkUtils::AvailableTerminal.first);
    mInstaller.setArguments({ SpkUtils::AvailableTerminal.second, "pkexec",
                              "bash", "-c",
                              QString("aptitude install '%1'; R=$?; read -p '%2'; exit $R")
                                .arg(pkgPath, prompt)});
    mInstaller.start();
  }
  else if(item == mActGdebi)
  {
    mInstaller.setProgram("pkexec");
    mInstaller.setArguments({ "gdebi", "-n", pkgPath });
    mInstaller.start();
  }
  else if(item == mActDeepinPkgInst)
  {
    mInstaller.setProgram("deepin-deb-installer");
    mInstaller.setArguments({ pkgPath });
    mInstaller.start();
  }
  else
    return Ignored;

  // Startup is very quick and we can risk blocking here a bit
  mInstaller.waitForStarted();
  return mInstaller.state() == QProcess::Running ? Succeeded : Failed;
}

void SpkPkgMgrApt::CheckInstallerAvailability()
{
  mActAptitudeTerm->setEnabled(QFile::exists("/usr/bin/aptitude"));
  mActGdebi->setEnabled(QFile::exists("/usr/bin/gdebi"));
  mActDeepinPkgInst->setEnabled(QFile::exists("/usr/bin/deepin-deb-installer"));
}

void SpkPkgMgrApt::InstallerExited(int exitCode, QProcess::ExitStatus status)
{
  if(status == QProcess::NormalExit)
    emit ReportInstallResult(mCurrentItemId, Succeeded, exitCode);
  else
    emit ReportInstallResult(mCurrentItemId, Failed, exitCode);
}


