
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

  mMenu->addAction(mActAptTerm);
  mMenu->addAction(mActAptitudeTerm);
  mMenu->addAction(mActGdebi);
  mMenu->addAction(mActDeepinPkgInst);
  mMenu->addSeparator();
  mMenu->addAction(mActDesc);
}

bool SpkPkgMgrApt::DetectRequirements()
{
  return QFile::exists("/usr/bin/apt") &&
      QFile::exists("/usr/bin/dpkg");
}

SpkPkgMgrBase::PkgInstallResult
SpkPkgMgrApt::ExecuteInstallation(QString pkgPath, int entryId)
{
  CheckInstallerAvailability();
  return SpkPkgMgrBase::ExecuteInstallation(pkgPath, entryId);
}

void SpkPkgMgrApt::CheckInstallerAvailability()
{
  mActAptitudeTerm->setEnabled(QFile::exists("/usr/bin/aptitude"));
  mActGdebi->setEnabled(QFile::exists("/usr/bin/gdebi"));
  mActDeepinPkgInst->setEnabled(QFile::exists("/usr/bin/deepin-deb-installer"));
}


