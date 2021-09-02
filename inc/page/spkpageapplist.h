
#pragma once

#include <QScrollArea>
#include <QList>
#include "spkresource.h"
#include "spkappitem.h"
#include "page/spkpagebase.h"
#include "spkstretchlayout.h"

namespace SpkUi
{
  class SpkPageAppList : public SpkPageBase
  {
      Q_OBJECT
    public:
      SpkPageAppList(QWidget *parent = nullptr);

      void AddApplicationEntry(QString name, QString pkgName, QString description, QString iconUrl,
                               int appId);
      void ClearAll();

    private:

    public:

    private:
      SpkStretchLayout *mItemLay;
      QList<SpkAppItem *> mAppItemList;

    signals:
      void ApplicationClicked(QString name, QString pkgName);

    public slots:
      void ResourceAcquisitionFinished(int id, ResourceResult result);
      void Activated();
  };
}
