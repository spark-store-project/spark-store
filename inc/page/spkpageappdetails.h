// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QFormLayout>
#include "page/spkpagebase.h"
#include "spkstretchlayout.h"
#include "spkimgviewer.h"

namespace SpkUi
{
  class SpkDetailEntry;

  class SpkClickLabel : public QLabel
  {
      Q_OBJECT
    protected:
      virtual void mousePressEvent(QMouseEvent *e) override { emit Pressed(); }
    signals:
      void Pressed();
  };

  class SpkPageAppDetails : public SpkPageBase
  {
      Q_OBJECT
    public:
      SpkPageAppDetails(QWidget *parent = nullptr);
      ~SpkPageAppDetails();

      void LoadAppResources(QString pkgName, QString icon, QStringList screenshots, QStringList tags);
      void SetWebsiteLink(QString url);
      void SetPackagePath(QString url);

    private:
      QString mPkgPath;
      QPixmap mBrokenImg, mIconLoading;

    public slots:
      void ResourceAcquisitionFinished(int id, ResourceResult result);
      void Activated();

    public:
      static constexpr QSize IconSize { 144, 144 };

      // Main Area
      QScrollArea *mMainArea, *mScreenshotArea;
      QWidget *mDetailWidget, *mIconTitleWidget, *mWid4MainArea, *mWid4ShotArea;
      QLabel *mAppTitle, *mAppIcon, *mAppDescription, *mAppShortDesc, *mPkgName, *mVersion,
             *mWebsite;
      SpkDetailEntry *mAuthor, *mContributor, *mSite, *mArch, *mSize;
      SpkStretchLayout *mDetailLay;
      QVBoxLayout *mDetailsLay, *mTitleLay, *mMainLay;
      QHBoxLayout *mIconTitleLay, *mScreenshotLay;
      QList<SpkClickLabel*> mScreenshotPreviews;

      QMap<int, QPixmap> mAppImages;
      SpkImgViewer *mImgViewer;

      // Bottom bar
      QWidget *mBottomBar;
      QPushButton *mBtnInstall, *mBtnDownload, *mBtnUninstall, *mBtnRequestUpdate, *mBtnReport;
      QHBoxLayout *mBottomBarLay;

    signals:
      void RequestDownload(QString name, QString pkgName, QString path);

    private slots:
      void ImageClicked();
  };

  class SpkDetailEntry : public QWidget
  {
      Q_OBJECT
    public:
      SpkDetailEntry(QWidget *parent = nullptr);
      void SetTitle(const QString &s) { mTitle.setText(s); }
      void SetValue(const QString &s) { mField.setText(s); }

    private:
      QLabel mTitle, mField;
      QHBoxLayout mLay;
  };
}
