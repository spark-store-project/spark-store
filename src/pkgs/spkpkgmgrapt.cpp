
#include <QFile>
#include "pkgs/spkpkgmgrapt.h"

SpkPkgMgrApt::SpkPkgMgrApt(QObject *parent) :
  SpkPkgMgrBase(parent)
{
  mActDesc = new QAction("Debian APT", this);
  mActDesc->setDisabled(true);

  mMenu->addSeparator();
  mMenu->addAction(mActDesc);
}

bool SpkPkgMgrApt::DetectRequirements()
{
  return QFile::exists("/usr/bin/apt") &&
      QFile::exists("/etc/apt/apt.conf");
}

SpkPkgMgrBase::PkgInstallResult
SpkPkgMgrApt::ExecuteInstallation(QString pkgPath, int entryId)
{
  return SpkPkgMgrBase::ExecuteInstallation(pkgPath, entryId);
}


