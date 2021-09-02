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
#include "inc/page/spkpageuitest.h"
#include "inc/page/spkpageapplist.h"

class QNetworkReply;

namespace SpkUi
{
  enum SpkStackedPages
  {
    PgInvalid = -1,
    PgAppList,
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
                this, &SpkSidebarSelector::ButtonPressed);
      }
      void BindCategoryWidget(QTreeWidget* w)
      {
        mCategoryWidget = w;
        connect(w, &QTreeWidget::itemPressed, this,
                &SpkSidebarSelector::TreeItemSelected);
      }
      void AddUnusableItem(QTreeWidgetItem *i) { mUnusableItems.append(i); }

    private slots:
      // We assume the objects in interest all have the correct properties
      void ButtonPressed(bool aBtnState)
      {
        auto b = qobject_cast<QPushButton*>(sender());
        if(mLastCheckedBtn)
        {
          if(mLastCheckedBtn != b)
          {
            mLastCheckedBtn->setChecked(false);
            mLastCheckedBtn = nullptr;
          }
          else
            b->setChecked(aBtnState);
        }
        else if(mLastSelectedItem)
        {
          mLastSelectedItem->setSelected(false);
          mLastSelectedItem = nullptr;
        }
        mLastCheckedBtn = b;
        emit SwitchToPage(b->property("spk_pageno").toInt());
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
        if(item->data(column, RoleItemIsCategory).toBool())
          emit SwitchToCategory(item->data(column, RoleItemCategoryPageId).toInt());
        else
          emit SwitchToPage(item->data(column, RoleItemCategoryPageId).toInt());
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
      void SwitchToCategory(int aCategoryId);
      void SwitchToPage(int aPageId);
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
      QPushButton *BtnSettings, *BtnFeedback, *BtnLogs;
      SpkSidebarTree *CategoryWidget;
      QMap<int, QTreeWidgetItem> *CategoryItemMap;
      SpkSidebarSelector *SidebarMgr;

      QTreeWidgetItem *CategoryParentItem;

      //Pages
      SpkPageUiTest *PageQssTest;
      SpkPageAppList *PageAppList;
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
                            mCategoryAppListGetReply;
    SpkUi::SpkStackedPages mCurrentPage = SpkUi::PgInvalid;

  public slots:
    void RefreshCategoryData();

  private slots:
    void SwitchToPage(SpkUi::SpkStackedPages page);

    void CategoryDataReceived();

    void EnterCategoryList(int id);
    void CategoryListDataReceived();

  private:
    void PopulateAppList(QJsonObject appData);
};
