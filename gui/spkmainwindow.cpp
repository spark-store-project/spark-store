
#include <QGuiApplication>
#include <QScreen>
#include <QJsonArray>
#include "spkmsgbox.h"
#include "spkmainwindow.h"
#include "spklogging.h"
#include "spkutils.h"
#include "spkuimsg.h"

SpkMainWindow::SpkMainWindow(QWidget *parent) : SpkWindow(parent)
{
  ui = new SpkUi::SpkMainWidget(parent);

  SetUseTitleBar(false);
  SetCentralWidget(ui);
  SetTitleBar(ui->TitleBar, false);
  RefreshCategoryData();

  auto size = QGuiApplication::primaryScreen()->size() * 0.25;
  resize(QGuiApplication::primaryScreen()->size() * 0.5);
  move(size.width(), size.height());
}

void SpkMainWindow::PopulateCategories(QJsonArray aCategoryData)
{
  using SpkUi::SpkSidebarSelector;
  QTreeWidgetItem *catg;
  auto w = ui->CategoryWidget;
  if(ui->CategoryParentItem->childCount()) // Clear all existing children if there is any
    foreach(auto &i, ui->CategoryParentItem->takeChildren())
      delete i;

  foreach(auto i, aCategoryData)
  {
    if(i.isObject())
    {
      auto j = i.toObject();
      double typeId;
      QString typeName;
      if(j.contains("type_id") && j.value("type_id").isDouble())
        typeId = j.value("type_id").toDouble();
      else goto WRONG_CATEGORY;
      if(j.contains("type_name") && j.value("type_name").isString())
        typeName = j.value("type_name").toString();
      else goto WRONG_CATEGORY;
      catg = new QTreeWidgetItem(ui->CategoryParentItem, QStringList(typeName));
      catg->setData(0, SpkSidebarSelector::RoleItemIsCategory, true);
      catg->setData(0, SpkSidebarSelector::RoleItemCategoryPageId, typeId);
      continue;
WRONG_CATEGORY:;
    }
    ui->CategoryParentItem->setExpanded(true);
  }
}

void SpkMainWindow::RefreshCategoryData()
{
  // Asynchronously call category API
  using namespace SpkUtils;
  VerifySingleRequest(mCategoryGetReply);
  mCategoryGetReply = STORE->SendApiRequest("type/get_type_list");
  DeleteReplyLater(mCategoryGetReply);
  connect(mCategoryGetReply, &QNetworkReply::finished, this, &SpkMainWindow::CategoryDataReceived);
}

void SpkMainWindow::CategoryDataReceived()
{
  QJsonValue retval;
  if(!SpkUtils::VerifyReplyJson(mCategoryGetReply, retval) || !retval.isArray())
  {
    sErr(tr("Failed to load categories!"));
    // TODO: Switch to an error page
  }
  PopulateCategories(retval.toArray());
}

SpkUi::SpkMainWidget::SpkMainWidget(QWidget *parent) : QFrame(parent)
{
  setObjectName("spk_mainwidget");

  Pager = new QStackedWidget(this);
  Pager->setObjectName("spk_mw_pager");
  Pager->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  TitleBar = new SpkTitleBar(this);
  TitleBar->setObjectName("spk_mw_titlebar");
  TitleBar->SetUseIcon(false);
  TitleBar->SetTitle("");

  VLayMain = new QVBoxLayout;
  VLayMain->setObjectName("spk_mw_main_vlay");
  VLayMain->setSpacing(0);
  VLayMain->setContentsMargins(0, 0, 0, 0);
  VLayMain->addWidget(TitleBar);
  VLayMain->addWidget(Pager);

  VLaySidebar = new QVBoxLayout;
  VLaySidebar->setObjectName("spk_mw_sidebar_lay");
  VLaySidebar->setSpacing(0);
  VLaySidebar->setContentsMargins(0, 0, 0, 0);

  SideBarRestrictor = new QWidget(this);
  SideBarRestrictor->setObjectName("spk_mw_sidebar_restrictor");
  SideBarRestrictor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  SideBarRestrictor->setMaximumWidth(200);
  SideBarRestrictor->setMinimumWidth(200);
  SideBarRestrictor->setLayout(VLaySidebar);

  HLaySideTop = new QHBoxLayout;
  HLaySideTop->setObjectName("spk_mw_sidebar_top_lay");
  HLaySideTop->setSpacing(8);
  HLaySideTop->setContentsMargins(8, 4, 4, 4);

  StoreIcon = new QLabel(this);
  StoreIcon->setObjectName("spk_mw_icon");
  StoreIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  StoreIcon->setMaximumSize({ 40, 40 });
  StoreIcon->setMinimumSize({ 40, 40 });
  StoreIcon->setPixmap(QIcon(":/icons/spark-store.svg").pixmap(StoreIcon->size()));

  SidebarMgr = new SpkSidebarSelector(this);
  SidebarMgr->setObjectName("spk_mw_sidebar_mgr");

  BtnSettings = new QPushButton(this);
  BtnSettings->setObjectName("styPlainChkBtn");
  BtnSettings->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  BtnSettings->setCheckable(true);
  BtnSettings->setMaximumSize({ 40, 40 });
  BtnSettings->setMinimumSize({ 40, 40 });
  BtnSettings->setIconSize(QSize(20, 20));
  BtnSettings->setIcon(SpkUi::GetThemedIcon("settings"));
  BtnSettings->setProperty("spk_pageno", 0);
  SidebarMgr->BindPageSwitcherButton(BtnSettings);

  HLaySideTop->addWidget(StoreIcon);
  HLaySideTop->addStretch();
  HLaySideTop->addWidget(BtnSettings);
  VLaySidebar->addLayout(HLaySideTop);

  using SpkUi::SpkSidebarSelector;
  CategoryWidget = new SpkSidebarTree(this);
  CategoryWidget->setObjectName("styMwCateg");
  CategoryWidget->setAutoFillBackground(true);
  CategoryWidget->setColumnCount(1);
  CategoryWidget->setHeaderHidden(true);
  CategoryWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  CategoryParentItem = new QTreeWidgetItem(QStringList(tr("Categories")));
  CategoryParentItem->setFlags(CategoryParentItem->flags().setFlag(Qt::ItemIsSelectable, false));
  CategoryParentItem->setExpanded(true);
  SidebarMgr->AddUnusableItem(CategoryParentItem);
  CategoryWidget->addTopLevelItem(CategoryParentItem);

  // FIXMEIFPOSSIBLE: Fusion adds extra gradient.
  // Details: https://forum.qt.io/topic/128190/fusion-style-kept-adding-an-extra-
  // layer-of-gradient-to-my-selected-item-of-qtreewidget-even-with-qss
  if(SpkUi::OldSystemStyle)
    CategoryWidget->setStyle(SpkUi::OldSystemStyle);
  VLaySidebar->addWidget(CategoryWidget);
  SidebarMgr->BindCategoryWidget(CategoryWidget);

  HorizontalDivide = new QHBoxLayout;
  HorizontalDivide->setObjectName("spk_mw_divide_hlay");
  HorizontalDivide->setSpacing(0);
  HorizontalDivide->setContentsMargins(0, 0, 0, 0);
  HorizontalDivide->setAlignment(Qt::AlignLeft);
  if(!SpkUi::States::IsUsingDtkPlugin)
    HorizontalDivide->addSpacing(SpkWindow::BorderWidth);
  HorizontalDivide->addWidget(SideBarRestrictor);
  HorizontalDivide->addLayout(VLayMain);

  // Initialize pages
  PageQssTest = new SpkUi::SpkPageUiTest(this);
  Pager->addWidget(PageQssTest);

  setLayout(HorizontalDivide);
}
