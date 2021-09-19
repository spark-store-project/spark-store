
#pragma once

#include <QtWidgets>
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
      void SetPageStatus(int total, int current, int itemCount, QString &keyword);
      void SetCurrentCategory(int categoryId) { mCategoryId = categoryId; }

    private:
      void DisablePageSwitchers();

    public:

    private:
      QVBoxLayout *mMainLay;
      QHBoxLayout *mPageSwitchLay;
      QPushButton *mBtnPgUp, *mBtnPgDown, *mBtnGotoPage;
      QLineEdit *mPageInput;
      QScrollArea *mAppsArea;
      QLabel *mPageIndicator;
      QWidget *mAppsWidget, *mPageSwitchWidget;
      SpkStretchLayout *mItemLay;
      QList<SpkAppItem *> mAppItemList;

      QIntValidator *mPageValidator;

      int mCategoryId, mCurrentPage;
      QString mKeyword;

    signals:
      void ApplicationClicked(int appId);
      void SwitchListPage(int categoryId, int page);
      void SwitchSearchPage(QString keyword, int page);

    public slots:
      void ResourceAcquisitionFinished(int id, ResourceResult result);
      void Activated();

    private slots:
      void PageUp();
      void PageDown();
      void GotoPage();
  };
}
