
#pragma once

#include "spkpkgmgrbase.h"
#include <QProcess>

class SpkPkgMgrApt final : public SpkPkgMgrBase
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
    void CheckInstallerAvailability();

  private slots:
    void InstallerExited(int, QProcess::ExitStatus);

  private:
    QAction *mActAptitudeTerm,
            *mActAptTerm,
            *mActGdebi,
            *mActDeepinPkgInst;

    QProcess mInstaller;

};
