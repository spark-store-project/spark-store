
#include "page/spkpageapplist.h"
#include "spkutils.h"
#include "spkuimsg.h"

namespace SpkUi
{
  SpkPageAppList::SpkPageAppList(QWidget *parent) : SpkPageBase(parent)
  {
    mAppsWidget = new QWidget;
    mAppsArea = new QScrollArea(this);
    mMainLay = new QVBoxLayout(this);
    mItemLay = new SpkStretchLayout(mAppsWidget);

    mPageSwitchWidget = new QWidget;
    mPageSwitchLay = new QHBoxLayout(mPageSwitchWidget);
    mBtnPgUp = new QPushButton;
    mBtnPgDown = new QPushButton;
    mBtnGotoPage = new QPushButton;
    mPageInput = new QLineEdit;
    mPageValidator = new QIntValidator(mPageInput);
    mPageIndicator = new QLabel;

    mPageValidator->setRange(1, 99);
    mPageInput->setFixedWidth(50);
    mPageInput->setValidator(mPageValidator);
    mBtnGotoPage->setText(tr("Goto"));
    mBtnPgUp->setText(tr("Previous"));
    mBtnPgDown->setText(tr("Next"));

    mPageSwitchLay->addWidget(mPageIndicator);
    mPageSwitchLay->addStretch();
    mPageSwitchLay->addWidget(mPageInput);
    mPageSwitchLay->addWidget(mBtnGotoPage);
    mPageSwitchLay->addWidget(mBtnPgUp);
    mPageSwitchLay->addWidget(mBtnPgDown);

    mAppsArea->setWidget(mAppsWidget);
    mAppsArea->setWidgetResizable(true);

    mMainLay->addWidget(mAppsArea);
    mMainLay->addWidget(mPageSwitchWidget);
    setLayout(mMainLay);

    connect(mBtnPgUp, &QPushButton::clicked, this, &SpkPageAppList::PageUp);
    connect(mBtnPgDown, &QPushButton::clicked, this, &SpkPageAppList::PageDown);
    connect(mBtnGotoPage, &QPushButton::clicked, this, &SpkPageAppList::GotoPage);
  }

  void SpkPageAppList::AddApplicationEntry(QString name, QString pkgName, QString description,
                                           QString iconUrl, int appId)
  {
    auto item = new SpkAppItem(appId, this);
    auto id = mAppItemList.size();

    connect(item, &SpkAppItem::clicked, this, &SpkPageAppList::ApplicationClicked);
    item->SetTitle(name);
    item->SetDescription(description);
    item->setProperty("pkg_name", pkgName);

    auto iconRes = RES->RequestResource(id, pkgName, SpkResource::ResourceType::AppIcon,
                                        iconUrl, 0);
    QPixmap icon;
    if(iconRes.status == SpkResource::ResourceStatus::Ready)
    {
      if(icon.loadFromData(iconRes.data))
        item->SetIcon(icon.scaled(SpkAppItem::IconSize_,
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation));
      else
      {
        item->SetIcon(QIcon(":/icons/broken-icon.svg").pixmap(SpkAppItem::IconSize_));
        RES->PurgeCachedResource(pkgName, SpkResource::ResourceType::AppIcon, 0);
      }
    }
    //TODO: prepare icons for loading entries
//  else
//    item->SetIcon(QPixmap(":/icons/loading_icon.svg").scaled(SpkAppItem::IconSize_));

    mAppItemList.append(item);
    mItemLay->addWidget(item);
  }

  void SpkPageAppList::ClearAll()
  {
    QWidget *itm;
    QLayoutItem *layitm;
    while((layitm = mItemLay->takeAt(0)))
    {
      itm = layitm->widget();
      itm->hide();
      itm->deleteLater();
    }
    mAppItemList.clear();
  }

  void SpkPageAppList::ResourceAcquisitionFinished(int id, ResourceResult result)
  {
    QPixmap icon;
    auto item = mAppItemList[id];
    if(result.status == SpkResource::ResourceStatus::Ready)
    {
      if(icon.loadFromData(result.data))
        item->SetIcon(icon.scaled(SpkAppItem::IconSize_,
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation));
      else
        item->SetIcon(QIcon(":/icons/broken-icon.svg").pixmap(SpkAppItem::IconSize_));
    }
    else if(result.status == SpkResource::ResourceStatus::Failed)
    {
      item->SetIcon(QIcon(":/icons/broken-icon.svg").pixmap(SpkAppItem::IconSize_));
      RES->PurgeCachedResource(item->property("pkg_name").toString(),
                               SpkResource::ResourceType::AppIcon, 0);
    }
  }

  void SpkPageAppList::SetPageStatus(int total, int current, int itemCount, QString &keyword)
  {
    mCurrentPage = current;
    mKeyword = keyword;
    mPageIndicator->setText(tr("Page %1 / %2, %3 apps in total")
                            .arg(current).arg(total).arg(itemCount));
    mBtnPgUp->setDisabled(current == 1);
    mBtnPgDown->setDisabled(total == current || total == 1);
    mBtnGotoPage->setDisabled(total == 1);
    mPageValidator->setTop(total);
  }

  void SpkPageAppList::DisablePageSwitchers()
  {
    mBtnPgDown->setDisabled(true);
    mBtnPgUp->setDisabled(true);
    mBtnGotoPage->setDisabled(true);
  }

  void SpkPageAppList::PageUp()
  {
    DisablePageSwitchers();
    if(mKeyword.isEmpty())
      emit SwitchListPage(mCategoryId, mCurrentPage - 1);
    else
      emit SwitchSearchPage(mKeyword, mCurrentPage - 1);
  }

  void SpkPageAppList::PageDown()
  {
    DisablePageSwitchers();
    if(mKeyword.isEmpty())
      emit SwitchListPage(mCategoryId, mCurrentPage + 1);
    else
      emit SwitchSearchPage(mKeyword, mCurrentPage + 1);
  }

  void SpkPageAppList::GotoPage()
  {
    if(mPageInput->text().isEmpty())
      return SpkUiMessage::SendStoreNotification(tr("Please enter page number to go to!"));
    int page = mPageInput->text().toInt();
    if(page > mPageValidator->top())
      return SpkUiMessage::SendStoreNotification(tr("Page %1 is not a valid page number!")
                                                 .arg(page));
    DisablePageSwitchers();
    if(mKeyword.isEmpty())
      emit SwitchListPage(mCategoryId, page);
    else
      emit SwitchSearchPage(mKeyword, page);
  }

  void SpkPageAppList::Activated()
  {
    RES->Acquire(this, false);
  }
}
