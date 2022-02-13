
#pragma once

#include "spkpkgmgrbase.h"

class SpkPkgMgrApt : public SpkPkgMgrBase
{
    Q_OBJECT

  public:
    SpkPkgMgrApt(QObject *parent = nullptr);

    static bool DetectRequirements();

    virtual PkgInstallResult ExecuteInstallation(QString pkgPath,
                                                 int entryId) override;

  // APT backend specific
    bool ChangeServerRepository(QString content);

  private:
    QAction *mActAptitudeTerm,
            *mActAptTerm,
            *mActGdebi,
            *mActDeepinPkgInst;

};
