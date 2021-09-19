// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QFormLayout>
#include "page/spkpagebase.h"
#include "spkstretchlayout.h"

namespace SpkUi
{
  class SpkDetailEntry;

  class SpkPageAppDetails : public SpkPageBase
  {
      Q_OBJECT
    public:
      SpkPageAppDetails(QWidget *parent = nullptr);

      void LoadAppResources(QString pkgName, QString icon, QStringList screenshots, QStringList tags);

    private:

    public slots:
      void ResourceAcquisitionFinished(int id, ResourceResult result);
      void Activated();

    public:
      static constexpr QSize IconSize { 144, 144 };

      QScrollArea *mMainArea;
      QWidget *mDetailWidget, *mIconTitleWidget, *mWid4MainArea;
      QLabel *mAppTitle, *mAppIcon, *mAppDescription, *mAppShortDesc, *mPkgName, *mVersion;
      SpkDetailEntry *mAuthor, *mContributor, *mSite, *mArch, *mSize;
      SpkStretchLayout *mDetailLay;
      QVBoxLayout *mMainLay, *mTitleLay, *mLay4MainArea;
      QHBoxLayout *mIconTitleLay;

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