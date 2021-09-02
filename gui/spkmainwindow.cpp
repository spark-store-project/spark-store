
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
  Initialize();

  SetUseTitleBar(false);
  SetCentralWidget(ui);
  SetTitleBar(ui->TitleBar, false);
  RefreshCategoryData();

  auto size = QGuiApplication::primaryScreen()->size() * 0.25;
  resize(QGuiApplication::primaryScreen()->size() * 0.5);
  move(size.width(), size.height());
}

void SpkMainWindow::SwitchToPage(SpkUi::SpkStackedPages page)
{
  if(mCurrentPage != page)
  {
    ui->Pager->setCurrentIndex(int(page));
    mCurrentPage = page;
    // If the page is a SpkPageBase (with a resource context), activate it for resource acquisition
    auto tryActivate = qobject_cast<SpkPageBase *>(ui->Pager->currentWidget());
    if(tryActivate)
      tryActivate->Activated();
  }
}

void SpkMainWindow::PopulateCategories(QJsonArray aCategoryData)
{
  using SpkUi::SpkSidebarSelector;
  QTreeWidgetItem *catg;
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
    return;
  }
  PopulateCategories(retval.toArray());
}

void SpkMainWindow::EnterCategoryList(int aCategoryId)
{
  // Asynchronously call category API
  using namespace SpkUtils;
  VerifySingleRequest(mCategoryAppListGetReply);
  QJsonObject reqData;
  QJsonDocument reqDoc;
  reqData.insert("type_id", QJsonValue(aCategoryId));
  reqDoc.setObject(reqData);
  mCategoryAppListGetReply = STORE->SendApiRequest("application/get_application_list", reqDoc);
  DeleteReplyLater(mCategoryAppListGetReply);
  connect(mCategoryAppListGetReply, &QNetworkReply::finished,
          this, &SpkMainWindow::CategoryListDataReceived);
  setCursor(Qt::BusyCursor);
}

void SpkMainWindow::CategoryListDataReceived()
{
  QJsonValue retval;
  if(!SpkUtils::VerifyReplyJson(mCategoryAppListGetReply, retval) || !retval.isObject())
  {
    sErrPop(tr("Failed to load app list of category! Type of retval: %1.").arg(retval.type()));
    return;
  }
  PopulateAppList(retval.toObject());
  setCursor(Qt::ArrowCursor);
  SwitchToPage(SpkUi::PgAppList);
}

void SpkMainWindow::PopulateAppList(QJsonObject appData)
{
  auto w = ui->PageAppList;
  w->ClearAll();

  if(!appData.contains("data") || !appData.value("data").isArray())
  {
    sErrPop(tr("Received invalid application list data!"));
    return;
  }

  auto apps = appData.value("data").toArray();

  foreach(auto i, apps)
  {
    if(i.isObject())
    {
      auto j = i.toObject();
      QString pkgName, displayName, description, iconPath;
      int appid;
      if(j.contains("package") && j.value("package").isString())
        pkgName = j.value("package").toString();
      else continue;
      if(j.contains("application_name_zh") && j.value("application_name_zh").isString())
        displayName = j.value("application_name_zh").toString();
      else continue;
      if(j.contains("description") && j.value("description").isString())
        description = j.value("description").toString();
      else continue;
      if(j.contains("application_id") && j.value("application_id").isDouble())
        appid = j.value("application_id").toInt();
      else continue;
      if(j.contains("icons") && j.value("icons").isString())
        iconPath = j.value("icons").toString();
      else continue;
      
      w->AddApplicationEntry(displayName, pkgName, description, iconPath, appid);
    }
  }
}

// ==================== Main Window Initialization ====================

void SpkMainWindow::Initialize()
{
  connect(ui->SidebarMgr, &SpkUi::SpkSidebarSelector::SwitchToCategory,
          this, &SpkMainWindow::EnterCategoryList);
}

// ==================== Main Widget Initialization ====================

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

  // Red-Black tree based map will be able to sort things
  QMap<SpkStackedPages, QWidget*> sorter;

  // Initialize pages
  PageAppList = new SpkUi::SpkPageAppList(this);
  PageAppList->setProperty("spk_pageid", SpkStackedPages::PgAppList);
  sorter[PgAppList] = PageAppList;

#ifndef NDEBUG // If only in debug mode should we initialize QSS test page
  PageQssTest = new SpkUi::SpkPageUiTest(this);
  PageQssTest->setProperty("spk_pageid", SpkStackedPages::PgQssTest);
  sorter[PgQssTest] = PageQssTest;
#endif

  for(auto i : sorter)
    Pager->addWidget(i);

  setLayout(HorizontalDivide);
}
