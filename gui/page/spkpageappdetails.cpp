// SPDX-License-Identifier: GPL-3.0-only

#include "page/spkpageappdetails.h"
#include "spkutils.h"
#include "spkappitem.h"

namespace SpkUi
{
  constexpr QSize SpkPageAppDetails::IconSize;

  void SpkPageAppDetails::LoadAppResources(QString aPkgName, QString aIcon, QStringList aScreenshots,
                                           QStringList aTags)
  {
    QPixmap pic;

    // Load icon
    auto res = RES->RequestResource(0, aPkgName, SpkResource::ResourceType::AppIcon, aIcon);
    if(res.status == SpkResource::ResourceStatus::Ready)
    {
      if(pic.loadFromData(res.data))
        mAppIcon->setPixmap(pic.scaled(IconSize,
                                       Qt::IgnoreAspectRatio,
                                       Qt::SmoothTransformation));
      else
      {
        mAppIcon->setPixmap(mBrokenImg);
        RES->PurgeCachedResource(aPkgName, SpkResource::ResourceType::AppIcon, 0);
      }
    }

    // Load screenshots. Screenshots have id starting with 1.
    if(aScreenshots.isEmpty())
      return;
    else
    {
      auto count = aScreenshots.size();
      mImgViewer->SetImageTotal(count);
      if(count > mScreenshotPreviews.size())
      {
        auto from = mScreenshotPreviews.size(), to = count - mScreenshotPreviews.size();
        for(int i = 0; i <= to; i++)
        {
          auto wid = new SpkClickLabel;
          wid->setProperty("shotId", from + i + 1);
          wid->setFixedHeight(200);
          wid->setCursor(Qt::PointingHandCursor);
          connect(wid, &SpkClickLabel::Pressed, this, &SpkPageAppDetails::ImageClicked);
          mScreenshotPreviews.append(wid);
          mScreenshotLay->addWidget(wid);
        }
      }
    }

    int shotId = 0;
    for(auto &i : aScreenshots)
    {
      shotId++;
      res = RES->RequestResource(shotId, aPkgName, SpkResource::ResourceType::AppScreenshot, i,
                                 shotId);
      auto preview = mScreenshotPreviews[shotId - 1];
      preview->setVisible(true);
      if(res.status == SpkResource::ResourceStatus::Ready)
      {
        if(pic.loadFromData(res.data))
        {
          mAppImages[shotId] = pic;
          mImgViewer->SetPixmap(shotId, &mAppImages[shotId]);
          preview->setPixmap(pic.scaledToHeight(200, Qt::SmoothTransformation));
        }
        else
        {
          mAppImages[shotId] = mBrokenImg;
          RES->PurgeCachedResource(aPkgName, SpkResource::ResourceType::AppScreenshot, 0);
        }
      }
      else
      {
        preview->setPixmap(mIconLoading);
      }
    }

    // TODO: tags
  }

  void SpkPageAppDetails::SetWebsiteLink(QString url)
  {
    mWebsite->setText(QString("<a href=\"%1\">%2</a>").arg(url, tr("Website link")));
  }

  void SpkPageAppDetails::SetPackagePath(QString url)
  {
    mPkgPath = url;
  }

  SpkPageAppDetails::SpkPageAppDetails(QWidget *parent) : SpkPageBase(parent),
    mBrokenImg(QIcon(":/icons/broken-icon.svg").pixmap(SpkAppItem::IconSize_)),
    mIconLoading(QIcon(":/icons/loading-icon.svg").pixmap(SpkAppItem::IconSize_))
  {
    mMainArea = new QScrollArea;
    mMainArea->setWidgetResizable(true);
    mMainArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mMainLay = new QVBoxLayout(this);
    mMainLay->addWidget(mMainArea);

    mBottomBar = new QWidget;
    mMainLay->addWidget(mBottomBar);

    mDetailsLay = new QVBoxLayout(mMainArea);
    mDetailsLay->setSizeConstraint(QLayout::SetMinAndMaxSize);

    mAppIcon = new QLabel;

    mAppTitle = new QLabel;
    mAppTitle->setObjectName("styDetTitle");
    mAppTitle->setWordWrap(true);

    mAppDescription = new QLabel;
    mAppDescription->setObjectName("styDetDesc");
    mAppDescription->setWordWrap(true);
    mAppShortDesc = new QLabel;
    mAppShortDesc->setObjectName("styDetDesc");
    mAppShortDesc->setWordWrap(true);
    // NOTE: Seems Qt have trouble dealing with wrapped text here. Removing the following operations
    // to mAppShortDesc will result in broken layout. Very possible that it's a Qt bug.
    mAppShortDesc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    mAppShortDesc->setMinimumWidth(100);
    mVersion = new QLabel;
    mVersion->setWordWrap(true);
    mWebsite = new QLabel;
    mPkgName = new QLabel;
    mPkgName->setObjectName("styDetPkg");
    mPkgName->setWordWrap(true);

    mTitleLay = new QVBoxLayout;
    mTitleLay->setAlignment(Qt::AlignTop);
    mTitleLay->addWidget(mAppTitle);
    mTitleLay->addWidget(mVersion);
    mTitleLay->addWidget(mAppShortDesc);
    mTitleLay->addWidget(mPkgName);
    mTitleLay->addWidget(mWebsite);
    mTitleLay->setSpacing(0);

    mIconTitleLay = new QHBoxLayout;
    mIconTitleLay->setAlignment(Qt::AlignLeft);
    mIconTitleLay->addWidget(mAppIcon);
    mIconTitleLay->addSpacing(15);
    mIconTitleLay->addLayout(mTitleLay);

    mIconTitleWidget = new QWidget;
    mIconTitleWidget->setLayout(mIconTitleLay);

    mAuthor = new SpkDetailEntry;
    mAuthor->SetTitle(tr("Author"));
    mContributor = new SpkDetailEntry;
    mContributor->SetTitle(tr("Contributor"));
//    mSite = new SpkDetailEntry;
//    mSite->SetTitle(tr("Website"));
    mArch = new SpkDetailEntry;
    mArch->SetTitle(tr("Architecture"));
    mSize = new SpkDetailEntry;
    mSize->SetTitle(tr("Size"));

    mDetailLay = new SpkStretchLayout;
    mDetailLay->setSpacing(12);
    mDetailLay->addWidget(mAuthor);
    mDetailLay->addWidget(mContributor);
    mDetailLay->addWidget(mSize);
    mDetailLay->addWidget(mArch);
//    mDetailLay->addWidget(mSite);

//    mDetailWidget = new QWidget;
//    mDetailWidget->setLayout(mDetailLay);
//    mDetailWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    mDetailsLay->setAlignment(Qt::AlignTop);
    mDetailsLay->addWidget(mIconTitleWidget);
    mDetailsLay->addLayout(mDetailLay);
    mDetailsLay->addWidget(mAppDescription);
//    mMainLay->addStretch();

    mScreenshotLay = new QHBoxLayout;
    mScreenshotArea = new QScrollArea;
    mWid4ShotArea = new QWidget;
    mWid4ShotArea->setLayout(mScreenshotLay);
    mScreenshotArea->setWidget(mWid4ShotArea);
    mScreenshotArea->setWidgetResizable(true);
    mScreenshotArea->setFixedHeight(230);
    mScreenshotArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mDetailsLay->addWidget(mScreenshotArea);

    mWid4MainArea = new QWidget;
    mWid4MainArea->setLayout(mDetailsLay);

    mMainArea->setWidget(mWid4MainArea);

    mWebsite->setTextFormat(Qt::RichText);
    mWebsite->setOpenExternalLinks(true);

    // Bottom bar buttons
    mBottomBarLay = new QHBoxLayout;
    mBottomBar->setLayout(mBottomBarLay);

    mBtnDownload = new QPushButton;
    mBtnDownload->setText(tr("Download"));

//    mBtnInstall = new QPushButton;
//    mBtnInstall->setText(tr("Install"));

    mBtnUninstall = new QPushButton;
    mBtnUninstall->setText(tr("Uninstall"));

    mBtnRequestUpdate = new QPushButton;
    mBtnRequestUpdate->setText(tr("Request Update"));

    mBtnReport = new QPushButton;
    mBtnReport->setText(tr("Report"));

    mBottomBarLay->addStretch();
    mBottomBarLay->addWidget(mBtnDownload);
//    mBottomBarLay->addWidget(mBtnInstall);
    mBottomBarLay->addWidget(mBtnUninstall);
    mBottomBarLay->addWidget(mBtnRequestUpdate);
    mBottomBarLay->addWidget(mBtnReport);

    mImgViewer = new SpkImgViewer;
    mImgViewer->setVisible(false);

    connect(mBtnDownload, &QPushButton::clicked,
            [=](){ emit RequestDownload(mAppTitle->text(), mPkgName->text(),
                                        "/store/reading/youdao-dict/youdao-dict_6.0.0-0~ubuntu_amd64.deb");
    });
  }

  SpkPageAppDetails::~SpkPageAppDetails()
  {
    delete mImgViewer;
  }

  void SpkPageAppDetails::ResourceAcquisitionFinished(int id, ResourceResult result)
  {
    QPixmap img;
//    qDebug() << "PageAppDetails: Resource" << id << "acquired";
    if(!id)
    {
      // id == 0, icon
      if(result.status == SpkResource::ResourceStatus::Ready)
      {
        if(img.loadFromData(result.data))
          mAppIcon->setPixmap(img.scaled(SpkAppItem::IconSize_,
                                          Qt::IgnoreAspectRatio,
                                          Qt::SmoothTransformation));
        else
          mAppIcon->setPixmap(mBrokenImg);
      }
      else if(result.status == SpkResource::ResourceStatus::Failed)
      {
        mAppIcon->setPixmap(mBrokenImg);
        RES->PurgeCachedResource(mPkgName->text(), SpkResource::ResourceType::AppIcon, 0);
      }
    }
    else
    {
      auto preview = mScreenshotPreviews[id - 1];
      preview->setVisible(true);
      if(result.status == SpkResource::ResourceStatus::Ready)
      {
        if(img.loadFromData(result.data))
        {
          mAppImages[id] = img;
          mImgViewer->SetPixmap(id, &mAppImages[id]);
          mScreenshotPreviews[id - 1]->setPixmap(img.scaledToHeight(200, Qt::SmoothTransformation));
        }
        else
        {
          mImgViewer->SetPixmap(id, &mBrokenImg);
          mScreenshotPreviews[id - 1]->setPixmap(mBrokenImg);
        }
      }
      else if(result.status == SpkResource::ResourceStatus::Failed)
      {
        mImgViewer->SetPixmap(id, &mBrokenImg);
        mScreenshotPreviews[id - 1]->setPixmap(mBrokenImg);
        RES->PurgeCachedResource(mPkgName->text(), SpkResource::ResourceType::AppIcon, 0);
      }
    }
  }

  void SpkPageAppDetails::Activated()
  {
    RES->Acquire(this, false);
    for(auto &i : mScreenshotPreviews)
      i->setVisible(false);
    mImgViewer->Clear();
  }

  void SpkPageAppDetails::ImageClicked()
  {
    mImgViewer->ShowWithImage(sender()->property("shotId").toInt());
  }

  SpkDetailEntry::SpkDetailEntry(QWidget *parent) : QWidget(parent)
  {
    setLayout(&mLay);
    mLay.addWidget(&mTitle);
    mLay.addWidget(&mField);
    mTitle.setAlignment(Qt::AlignLeft);
    mField.setAlignment(Qt::AlignRight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumWidth(300);
    setAutoFillBackground(true);
  }
}
