
#pragma once

#include "spkpkgmgrbase.h"

class SpkPkgMgrPacman final : public SpkPkgMgrBase
{
    Q_OBJECT

  public:
    SpkPkgMgrPacman(QObject *parent = nullptr);

    static bool DetectRequirements();

    virtual PkgInstallResult ExecuteInstallation(QString pkgPath,
                                                 int entryId) override;



};
