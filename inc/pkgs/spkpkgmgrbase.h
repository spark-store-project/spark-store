
#pragma once

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QUrl>
#include <QDesktopServices>

#include "spkutils.h"

class SpkPkgMgrBase : public QObject
{
    Q_OBJECT

  public:
    SpkPkgMgrBase(QObject *parent = nullptr) : QObject(parent)
    {
      Q_ASSERT(mInstance == nullptr);
      mInstance = this;

      mActOpen = new QAction(tr("Open package"), this);
      mActOpenDir = new QAction(tr("Open containing directory"), this);
      mMenu = new QMenu(tr("Package Actions"));
      mMenu->addAction(mActOpen);
      mMenu->addAction(mActOpenDir);
      mMenu->setAttribute(Qt::WA_TranslucentBackground);
      mMenu->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    }

    enum PkgInstallResult
    {
      Failed,
      Succeeded,
      Ignored ///< No installation action is taken, no messages be displayed
    };

    /**
     * @brief Detects if this system qualified to use this kind of packaging
     */
    static bool DetectRequirements() { return true; }

    /**
     * @brief Popup a menu at cursor to select installation methods
     *        and do installation accordingly
     * @param pkgPath Path to the package file
     * @param entryId ID of the download entry
     */
    virtual PkgInstallResult ExecuteInstallation(QString pkgPath, int entryId)
    {
      Q_UNUSED(entryId);
      auto item = mMenu->exec(QCursor::pos());
      if(item == mActOpenDir)
        QDesktopServices::openUrl(QUrl(SpkUtils::CutPath(pkgPath)));
      else if(item == mActOpen)
        QDesktopServices::openUrl(QUrl(pkgPath));
      return Ignored;
    }

    /**
     * @brief Called when Spark Store installs a software when it's running
     *        in CLI mode
     * @param pkgPath Path to the package file
     */
    virtual PkgInstallResult CliInstall(QString pkgPath)
    {
      qInfo() << tr("Spark Store cannot install your package because no supported "
                    "packaging system has been found. You shall decide what you "
                    "want to do with the downloaded package.\n\n"
                    "File path:")
              << pkgPath;
      return Ignored;
    }

  public:
    static SpkPkgMgrBase *Instance() { return mInstance; }

  protected:
    QAction *mActOpenDir, *mActOpen, *mActDesc;
    QMenu *mMenu;
    int mCurrentItemId; ///< ID of currently installing download item

  private:
    static SpkPkgMgrBase *mInstance;

  signals:
    void ReportInstallResult(int entryId,
                             SpkPkgMgrBase::PkgInstallResult result,
                             int exitCode);
};
