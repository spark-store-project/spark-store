//
// Created by rigoligo on 2021/5/9.
//

#pragma once

#include "spkwindow.h"
#include <vector>
#include <QTextEdit>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QJsonObject>
#include "spksidebartree.h" // In place of #include <QTreeWidget>
#include <QPointer>
#include <QTimeLine>
#include <QQueue>
#include "spkfocuslineedit.h"
#include "spkiconbutton.h"
#include "page/spkpageuitest.h"
#include "page/spkpagehome.h"
#include "page/spkpageapplist.h"
#include "page/spkpageappdetails.h"
#include "page/spkpagedownloads.h"
#include "page/spkpagesettings.h"

class QNetworkReply;

namespace SpkUi
{
  enum SpkStackedPages
  {
    PgInvalid = -1,
    PgHomepage,
    PgAppList,
    PgAppDetails,
    PgDownloads,
    PgSettings,
    PgQssTest // Must be at last
  };

  const std::vector<SpkStackedPages> ResourceContexts
  {
    PgAppList
  };

  class SpkSidebarSelector : public QObject
  {
      Q_OBJECT
    private:
      QPushButton *mLastCheckedBtn;
      QTreeWidgetItem *mLastSelectedItem;
      QTreeWidget *mCategoryWidget;
      QVector<QTreeWidgetItem *> mUnusableItems; // Unselectable top level items; never changes

      QTreeWidgetItem* mLastCategoryItem;
      int mLastCategoryPage;

    public:
      SpkSidebarSelector(QObject *parent = nullptr) : QObject(parent)
      {
        mLastCheckedBtn = nullptr;
        mLastSelectedItem = nullptr;
      }
      // Tree item can either represent a page or a category, data of special roles
      // help identify them; Buttons instead can only represent a page
      constexpr static int RoleItemIsCategory = Qt::UserRole + 1;
      constexpr static int RoleItemCategoryPageId= Qt::UserRole + 2;
      void BindPageSwitcherButton(QAbstractButton* w)
      {
        connect(w, &QAbstractButton::toggled,
                this, &SpkSidebarSelector::ButtonToggled);
        connect(w, &QAbstractButton::pressed,
                this, &SpkSidebarSelector::ButtonPressed);
      }
      void BindCategoryWidget(QTreeWidget* w)
      {
        mCategoryWidget = w;
        connect(w, &QTreeWidget::itemPressed, this,
                &SpkSidebarSelector::TreeItemSelected);
      }
      void AddUnusableItem(QTreeWidgetItem *i) { mUnusableItems.append(i); }
      void GoBack()
      {
        emit SwitchToCategory(mLastCategoryPage, 0);
        mLastCategoryItem->setSelected(true);
      }

    private slots:
      // We assume the objects in interest all have the correct properties
      void ButtonToggled(bool aBtnState)
      {
        auto b = qobject_cast<QPushButton*>(sender());
        if(mLastCheckedBtn)
        {
          if(mLastCheckedBtn != b)
          {
            mLastCheckedBtn->setChecked(false);
            mLastCheckedBtn = nullptr;
          }
        }
        else if(mLastSelectedItem)
        {
          mLastSelectedItem->setSelected(false);
          mLastSelectedItem = nullptr;
        }
        mLastCheckedBtn = b;
        auto id = b->property("spk_pageno").toInt();
        emit SwitchToPage(static_cast<SpkStackedPages>(id));
      }
      void ButtonPressed()
      { // Prevent a selected button from being deselected by clicking on it
        auto b = qobject_cast<QPushButton*>(sender());
        if(mLastCheckedBtn == b)
          b->setChecked(false);
      }
      void TreeItemSelected(QTreeWidgetItem *item, int column)
      {
        if(mUnusableItems.contains(item))
        {
          UnusableItemSelected(item); return;
        }
        if(mLastCheckedBtn)
        {
          mLastCheckedBtn->setChecked(false);
          mLastCheckedBtn = nullptr;
        }
        mLastSelectedItem = item;
        auto id = item->data(column, RoleItemCategoryPageId).toInt();
        if(item->data(column, RoleItemIsCategory).toBool())
          emit SwitchToCategory(id, 0), mLastCategoryPage = id, mLastCategoryItem = item;
        else
          emit SwitchToPage(static_cast<SpkStackedPages>(id));
      }
      void UnusableItemSelected(QTreeWidgetItem *i)
      {
        i->setSelected(false);
        if(mLastSelectedItem)
        {
          mLastSelectedItem->setSelected(true);
        }
        else if(mLastCheckedBtn)
        {
          mLastCheckedBtn->setChecked(true);
        }
      }

    signals:
      void SwitchToCategory(int aCategoryId, int aPage);
      void SwitchToPage(SpkStackedPages aPageId);
  };

  class SpkMainWidget : public QFrame
  {
      Q_OBJECT

    public:
      SpkMainWidget(QWidget *parent = nullptr);

      QHBoxLayout *HorizontalDivide;
      QVBoxLayout *VLayMain;
      SpkTitleBar *TitleBar;

      QStackedWidget *Pager;

      // Category widget is for switching pages
      QWidget *SideBarRestrictor;
      QVBoxLayout *VLaySidebar;
      QHBoxLayout *HLaySideTop;
      QLabel *StoreIcon;
      SpkIconButton *BtnSettings, *BtnFeedback, *BtnLogs, *BtnDayNight, *BtnBack;
      SpkSidebarTree *CategoryWidget;
      QMap<int, QTreeWidgetItem> *CategoryItemMap;
      SpkSidebarSelector *SidebarMgr;

      QTreeWidgetItem *HomepageItem,
                      *CategoryParentItem,
                      *AppDetailsItem,
                      *DownloadsItem,
                      *UiTestItem;

      // Title bar search bar
      SpkFocusLineEdit *SearchEdit;
      QAction *ActClearSearchBar, *ActSearchIcon;
      QTimeLine *SearchBarAnim;

      //Pages
      SpkPageUiTest *PageQssTest;

      SpkPageHome *PageHome;
      SpkPageAppList *PageAppList;
      SpkPageAppDetails *PageAppDetails;
      SpkPageDownloads *PageDownloads;
      SpkPageSettings *PageSettings;
  };
}

class SpkMainWindow : public SpkWindow
{
    Q_OBJECT
  private:
    SpkUi::SpkMainWidget *ui;

  public:
    SpkMainWindow(QWidget *parent = nullptr);
    
    void PopulateCategories(QJsonArray);

  private:
    void Initialize();

  private:
    QPointer<QNetworkReply> mCategoryGetReply,
                            mCategoryAppListGetReply,
                            mAppDetailsGetReply;
    SpkUi::SpkStackedPages mCurrentPage = SpkUi::PgInvalid;
    QList<QPair<SpkIconButton*, QString>> mThemedUiIconReferences;

  public slots:
    void ReloadThemedUiIcons();
    void RefreshCategoryData();

  private slots:
    void SwitchDayNightTheme();

    void SwitchToPage(SpkUi::SpkStackedPages page);

    void CategoryDataReceived();
    // Enter a category (and switch pages)
    void EnterCategoryList(int aCategoryId, int aPage);
    void CategoryListDataReceived();
    // Search a keyword (and switch pages)
    void SearchKeyword(QString aKeyword, int aPage);
    void SearchDataReceived();
    // Enter the details page of an application (and switch pages)
    void EnterAppDetails(int aAppId);
    void AppDetailsDataReceived();

  private:
    void PopulateAppList(QJsonObject appData, QString &&keyword);
    void PopulateAppDetails(QJsonObject appDetails);
};
