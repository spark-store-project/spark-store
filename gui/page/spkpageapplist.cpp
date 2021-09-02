
#include <page/spkpageapplist.h>
#include "inc/page/spkpageapplist.h"
#include "spkutils.h"

namespace SpkUi
{
  SpkPageAppList::SpkPageAppList(QWidget *parent) : SpkPageBase(parent)
  {
    mItemLay = new SpkStretchLayout(this);
    mItemLay->setContentsMargins(6, 6, 6, 6);
  }

  void SpkPageAppList::AddApplicationEntry(QString name, QString pkgName, QString description,
                                           QString iconUrl, int appId)
  {
    auto item = new SpkAppItem(appId, this);
    auto id = mAppItemList.size();

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
        item->SetIcon(QIcon::fromTheme("dialog-error").pixmap(SpkAppItem::IconSize_));
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
        item->SetIcon(QIcon::fromTheme("dialog-error").pixmap(SpkAppItem::IconSize_));
    }
    else if(result.status == SpkResource::ResourceStatus::Failed)
    {
      item->SetIcon(QIcon::fromTheme("dialog-error").pixmap(SpkAppItem::IconSize_));
      RES->PurgeCachedResource(item->property("pkg_name").toString(),
                               SpkResource::ResourceType::AppIcon, 0);
    }
  }

  void SpkPageAppList::Activated()
  {
    RES->Acquire(this, false);
  }
}
