
#include "pkgs/spkpkgmgrpacman.h"

#include <QFile>

SpkPkgMgrBase *SpkPkgMgrBase::mInstance = nullptr;

SpkPkgMgrPacman::SpkPkgMgrPacman(QObject *parent) :
  SpkPkgMgrBase(parent)
{
  mActDesc = new QAction(tr("ArchLinux Pacman"), this);
  mActDesc->setEnabled(false);

  mMenu->addSeparator();
  mMenu->addAction(mActDesc);
}

bool SpkPkgMgrPacman::DetectRequirements()
{
  return QFile::exists("/usr/bin/pacman") &&
         QFile::exists("/etc/pacman.conf");
}

SpkPkgMgrBase::PkgInstallResult
SpkPkgMgrPacman::ExecuteInstallation(QString pkgPath, int entryId)
{
  SpkPkgMgrBase::ExecuteInstallation(pkgPath, entryId);
}
