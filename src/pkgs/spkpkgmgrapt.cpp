
#include <QFile>
#include "pkgs/spkpkgmgrapt.h"

SpkPkgMgrApt::SpkPkgMgrApt(QObject *parent) :
  SpkPkgMgrBase(parent)
{

}

bool SpkPkgMgrApt::DetectRequirements()
{
  return QFile::exists("/usr/bin/apt") &&
      QFile::exists("/etc/apt/apt.conf");
}

SpkPkgMgrBase::PkgInstallResult
SpkPkgMgrApt::ExecuteInstallation(QString pkgPath, int entryId)
{

}


