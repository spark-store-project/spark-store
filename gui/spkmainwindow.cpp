
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
  ui = new SpkUi::SpkMainWidget(this);
  Initialize();

  SetCentralWidget(ui);
  RefreshCategoryData();
  GetTitleBar()->SetTitle("");
  GetTitleBar()->SetUseIcon(true);
  GetTitleBar()->SetIcon(QIcon(":/icons/spark-store.svg").pixmap({ 40, 40 }));

  auto size = QGuiApplication::primaryScreen()->size() * 0.5;
  size = size.expandedTo(QSize(900, 600));
  resize(size);
  auto pos = QGuiApplication::primaryScreen()->size() * 0.5 - size * 0.5;
  move(pos.width(), pos.height());
}

void SpkMainWindow::SwitchDayNightTheme()
{
  if(SpkUi::CurrentStyle == SpkUi::Dark)
    SpkUi::SetGlobalStyle(SpkUi::Light, true);
  else
    SpkUi::SetGlobalStyle(SpkUi::Dark, true);
  ReloadThemedUiIcons();
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

    ui->BtnBack->setVisible(page == SpkUi::SpkStackedPages::PgAppDetails);
    ui->BtnBack->setEnabled(true);
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
  auto verify = SpkUtils::VerifyReplyJson(mCategoryGetReply, retval);
  if(verify || !retval.isArray())
  {
    sErr(tr("Failed to load categories! Type=%1 Code=%2").arg(retval.type()).arg(verify));
    sNotify(tr("Cannot load categories! Type: %1 Code: %2").arg(retval.type()).arg(verify));
    ui->PageHome->ui->widReloadCategory->setVisible(true);
    return;
  }
  ui->PageHome->ui->widReloadCategory->setVisible(false);
  PopulateCategories(retval.toArray());
}

void SpkMainWindow::EnterCategoryList(int aCategoryId, int aPage)
{
  // Asynchronously call category API
  using namespace SpkUtils;
  VerifySingleRequest(mCategoryAppListGetReply);
  QJsonObject reqData;
  QJsonDocument reqDoc;
  reqData.insert("type_id", QJsonValue(aCategoryId));
  reqData.insert("page", QJsonValue(aPage));
  reqDoc.setObject(reqData);
  mCategoryAppListGetReply = STORE->SendApiRequest("application/get_application_list", reqDoc);
  DeleteReplyLater(mCategoryAppListGetReply);
  connect(mCategoryAppListGetReply, &QNetworkReply::finished,
          this, &SpkMainWindow::CategoryListDataReceived);
  setCursor(Qt::BusyCursor);
  ui->PageAppList->SetCurrentCategory(aCategoryId); // AppList needs to remember current category
}

void SpkMainWindow::CategoryListDataReceived()
{
  QJsonValue retval;
  setCursor(Qt::ArrowCursor);
  int verify = SpkUtils::VerifyReplyJson(mCategoryAppListGetReply, retval);
  if(verify || !retval.isObject())
  {
    sErr(tr("Failed to load app list of category! Type=%1 Code=%2").arg(retval.type()).arg(verify));
    sNotify(tr("Failed to load app list of category! Type: %1 Code: %2").arg(retval.type()).arg(verify));
    return;
  }
  SwitchToPage(SpkUi::PgAppList);
  PopulateAppList(retval.toObject(), "");
}

void SpkMainWindow::SearchKeyword(QString aKeyword, int aPage)
{
  using namespace SpkUtils;
  VerifySingleRequest(mCategoryAppListGetReply);
  QJsonObject reqData;
  QJsonDocument reqDoc;
  reqData.insert("application_name", QJsonValue(aKeyword));
  reqData.insert("page", QJsonValue(aPage));
  reqDoc.setObject(reqData);
  mCategoryAppListGetReply = STORE->SendApiRequest("application/get_application_list", reqDoc);
  mCategoryAppListGetReply->setProperty("keyword", aKeyword);
  DeleteReplyLater(mCategoryAppListGetReply);
  connect(mCategoryAppListGetReply, &QNetworkReply::finished,
          this, &SpkMainWindow::SearchDataReceived);
  setCursor(Qt::BusyCursor);
}

void SpkMainWindow::SearchDataReceived()
{
  QJsonValue retval;
  setCursor(Qt::ArrowCursor);
  auto verify = SpkUtils::VerifyReplyJson(mCategoryAppListGetReply, retval);
  if(verify || !retval.isObject())
  {
    sErr(tr("Failed to search keyword! Type=%1 Code=%2").arg(retval.type()).arg(verify));
    sNotify(tr("Failed to search keyword! Type: %1 Code: %2").arg(retval.type()).arg(verify));
    return;
  }
  SwitchToPage(SpkUi::PgAppList);
  PopulateAppList(retval.toObject(), mCategoryAppListGetReply->property("keyword").toString());
}

void SpkMainWindow::PopulateAppList(QJsonObject appData, QString &&keyword)
{
  auto w = ui->PageAppList;
  w->ClearAll();
  static auto err =
  [](){
    sErr("Received invalid application list data!");
    SpkUiMessage::SendStoreNotification(tr("Received an invalid response. Please try again!"));
    return;
  };
  int pgCurrent, pgTotal, totalApps;

  if(appData.contains("currentPage") && appData.value("currentPage").isDouble())
    pgCurrent = appData.value("currentPage").toInt();
  else return err();
  if(appData.contains("totalPages") && appData.value("totalPages").isDouble())
    pgTotal = appData.value("totalPages").toInt();
  else return err();
  if(appData.contains("count") && appData.value("count").isDouble())
    totalApps = appData.value("count").toInt();
  else return err();
  w->SetPageStatus(pgTotal, pgCurrent, totalApps, keyword);

  if(!appData.contains("data") || !appData.value("data").isArray())
    return err();

  auto apps = appData.value("data").toArray();

  for(auto &&i : apps)
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

void SpkMainWindow::EnterAppDetails(int aAppId)
{
  using namespace SpkUtils;
  VerifySingleRequest(mAppDetailsGetReply);
  QJsonObject reqData;
  QJsonDocument reqDoc;
  reqData.insert("application_id", QJsonValue(aAppId));
  reqDoc.setObject(reqData);
  mAppDetailsGetReply = STORE->SendApiRequest("application/get_application_detail", reqDoc);
  DeleteReplyLater(mAppDetailsGetReply);
  connect(mAppDetailsGetReply, &QNetworkReply::finished,
          this, &SpkMainWindow::AppDetailsDataReceived);
  setCursor(Qt::BusyCursor);
}

void SpkMainWindow::AppDetailsDataReceived()
{
  QJsonValue retval;
  setCursor(Qt::ArrowCursor);
  auto verify = SpkUtils::VerifyReplyJson(mAppDetailsGetReply, retval);
  if(verify || !retval.isObject())
  {
    sErr(tr("Failed to open app details page! Type=%1 Code=%2").arg(retval.type()).arg(verify));
    sNotify(tr("Failed to open app details page! Type: %1 Code: %2").arg(retval.type()).arg(verify));
    return;
  }
  SwitchToPage(SpkUi::PgAppList);
  PopulateAppDetails(retval.toObject());
}

void SpkMainWindow::PopulateAppDetails(QJsonObject appDetails)
{
  QString pkgName, author, contributor, site, iconPath, arch, version, details, shortDesc, name,
          pkgPath;
  QStringList screenshots, tags;
  int packageSize;
  static auto err =
  [](){
    sErr("Received invalid application details!");
    SpkUiMessage::SendStoreNotification(tr("Received an invalid response. Please try again!"));
    return;
  };

  if(appDetails.contains("package") && appDetails.value("package").isString())
    pkgName = appDetails.value("package").toString();
  else return err();
  if(appDetails.contains("application_name_zh") && appDetails.value("application_name_zh").isString())
    name = appDetails.value("application_name_zh").toString();
  else name = pkgName;
  if(appDetails.contains("version") && appDetails.value("version").isString())
    version = appDetails.value("version").toString();
  else return err();
  if(appDetails.contains("icons") && appDetails.value("icons").isString())
    iconPath= appDetails.value("icons").toString();
  if(appDetails.contains("author") && appDetails.value("author").isString())
    author = appDetails.value("author").toString();
  if(appDetails.contains("contributor") && appDetails.value("contributor").isString())
    contributor = appDetails.value("contributor").toString();
  if(appDetails.contains("website") && appDetails.value("website").isString())
    site = appDetails.value("website").toString();
  if(appDetails.contains("description") && appDetails.value("description").isString())
    shortDesc = appDetails.value("description").toString();
  if(appDetails.contains("more") && appDetails.value("more").isString())
    details = appDetails.value("more").toString();
  if(appDetails.contains("arch") && appDetails.value("arch").isString())
    arch = appDetails.value("arch").toString();
  if(appDetails.contains("size") && appDetails.value("size").isDouble())
    packageSize = appDetails.value("size").toInt();
  if(appDetails.contains("deb_url") && appDetails.value("deb_url").isString())
    pkgPath = appDetails.value("deb_url").toString();

  QJsonArray imgs;
  if(appDetails.contains("img_urls") && appDetails.value("img_urls").isArray())
    imgs = appDetails.value("img_urls").toArray();
  if(!imgs.isEmpty())
    for(auto &&i : imgs)
      if(i.isString()) screenshots << i.toString();

  QJsonArray tags_j;
  if(appDetails.contains("tags") && appDetails.value("tags").isArray())
    imgs = appDetails.value("tags").toArray();
  if(!tags_j.isEmpty())
    for(auto &&i : tags_j)
      if(i.isString()) tags << i.toString();

  // Details string has a strangely appended LF. IDK but still should remove it.
  shortDesc = shortDesc.trimmed();

  auto w = ui->PageAppDetails;
  w->mPkgName->setText(pkgName);
  w->mAppTitle->setText(name);
  w->mAppShortDesc->setText(shortDesc);
  w->mAppDescription->setText(details);
  w->mAuthor->SetValue(author);
  w->mContributor->SetValue(contributor);
//  w->mSite->SetValue(site); // Doesn't look good, I disabled it temporarily. Better solution?
  w->SetWebsiteLink(site);
  w->mArch->SetValue(arch);
  w->mSize->SetValue(SpkUtils::BytesToSize(packageSize));
  w->mVersion->setText(version);
  w->SetPackagePath(pkgPath);
  SwitchToPage(SpkUi::PgAppDetails);
  ui->AppDetailsItem->setHidden(false);
  ui->CategoryWidget->setCurrentItem(ui->AppDetailsItem);
  w->LoadAppResources(pkgName, iconPath, screenshots, tags);
}

void SpkMainWindow::ReloadThemedUiIcons()
{
  for(auto &i : mThemedUiIconReferences)
    i.first->SetIcon(SpkUi::GetThemedIcon(i.second), QSize { 20, 20 });
}

// ==================== Main Window Initialization ====================

void SpkMainWindow::Initialize()
{
  connect(ui->SidebarMgr, &SpkUi::SpkSidebarSelector::SwitchToPage,
          this, &SpkMainWindow::SwitchToPage);
  connect(ui->SidebarMgr, &SpkUi::SpkSidebarSelector::SwitchToCategory,
          this, &SpkMainWindow::EnterCategoryList);
  connect(ui->PageAppList, &SpkUi::SpkPageAppList::SwitchListPage,
          this, &SpkMainWindow::EnterCategoryList);
  connect(ui->PageAppList, &SpkUi::SpkPageAppList::SwitchSearchPage,
          this, &SpkMainWindow::SearchKeyword);
  connect(ui->SearchEdit, &QLineEdit::returnPressed,
          [=](){ emit SearchKeyword(ui->SearchEdit->text(), 1); });
  connect(ui->PageAppList, &SpkUi::SpkPageAppList::ApplicationClicked,
          this, &SpkMainWindow::EnterAppDetails);
  connect(ui->BtnDayNight, &QPushButton::clicked,
          this, &SpkMainWindow::SwitchDayNightTheme);
  if(SpkUi::States::IsUsingDtkPlugin)
  {
    connect(SpkUi::DtkPlugin, &SpkDtkPlugin::DarkLightThemeChanged,
            this, &SpkMainWindow::ReloadThemedUiIcons);
  }
  connect(ui->PageAppDetails, &SpkUi::SpkPageAppDetails::RequestDownload,
          ui->PageDownloads, &SpkUi::SpkPageDownloads::AddDownloadTask);
  connect(ui->PageHome->ui->btnReloadCategory, &QPushButton::clicked,
          this, &SpkMainWindow::RefreshCategoryData);

  // Register themed button icons
//  mThemedUiIconReferences.append({ ui->BtnSettings, "settings" });
  mThemedUiIconReferences.append({ ui->BtnDayNight, "daynight" });

  ReloadThemedUiIcons();
}

// ==================== Main Widget Initialization ====================

SpkUi::SpkMainWidget::SpkMainWidget(QWidget *parent) : QFrame(parent)
{
  setObjectName("spk_mainwidget");

  Pager = new QStackedWidget(this);
  Pager->setObjectName("spk_mw_pager");
  Pager->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  SidebarMgr = new SpkSidebarSelector(this);
  SidebarMgr->setObjectName("spk_mw_sidebar_mgr");

  BtnSettings = new SpkIconButton(this);
  BtnSettings->setObjectName("styPlainChkBtn");
  BtnSettings->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  BtnSettings->setCheckable(true);
  BtnSettings->setFixedSize({ 40, 40 });
  BtnSettings->SetIcon(QIcon(":/icons/settings.svg"), QSize(20, 20));
  BtnSettings->setProperty("spk_pageno", PgSettings);
  SidebarMgr->BindPageSwitcherButton(BtnSettings);

  BtnDayNight = new SpkIconButton(this);
  BtnDayNight->setObjectName("styPlainChkBtn");
  BtnDayNight->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  BtnDayNight->setFixedSize({ 40, 40 });
  BtnDayNight->SetIcon(QIcon(":/icons/daynight.svg"), QSize(20, 20));

  BtnBack = new SpkIconButton(this);
  BtnBack->setFixedSize({ 40, 40 });
  BtnBack->SetIcon(QIcon(":/icons/back.svg"), QSize(20, 20));
  BtnBack->setVisible(false);

  using SpkUi::SpkSidebarSelector;
  CategoryWidget = new SpkSidebarTree(this);
  CategoryWidget->setObjectName("styMwCateg");
  CategoryWidget->setAutoFillBackground(true);
  CategoryWidget->setColumnCount(1);
  CategoryWidget->setHeaderHidden(true);
  CategoryWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  CategoryWidget->setFixedWidth(250);

  //============ Sidebar entries BEGIN ============
  HomepageItem = new QTreeWidgetItem(QStringList(tr("Home")));
  HomepageItem->setData(0, SpkSidebarSelector::RoleItemIsCategory, false);
  HomepageItem->setData(0, SpkSidebarSelector::RoleItemCategoryPageId, SpkStackedPages::PgHomepage);

  AppDetailsItem = new QTreeWidgetItem(QStringList(tr("App Details")));
  AppDetailsItem->setData(0, SpkSidebarSelector::RoleItemIsCategory, false);
  AppDetailsItem->setData(0, SpkSidebarSelector::RoleItemCategoryPageId, SpkStackedPages::PgAppDetails);

  CategoryParentItem = new QTreeWidgetItem(QStringList(tr("Categories")));
  CategoryParentItem->setFlags(CategoryParentItem->flags().setFlag(Qt::ItemIsSelectable, false));

  DownloadsItem = new QTreeWidgetItem(QStringList(tr("Downloads")));
  DownloadsItem->setData(0, SpkSidebarSelector::RoleItemIsCategory, false);
  DownloadsItem->setData(0, SpkSidebarSelector::RoleItemCategoryPageId, SpkStackedPages::PgDownloads);
#ifndef NDEBUG
  UiTestItem = new QTreeWidgetItem(QStringList(tr("UI TEST")));
  UiTestItem->setData(0, SpkSidebarSelector::RoleItemIsCategory, false);
  UiTestItem->setData(0, SpkSidebarSelector::RoleItemCategoryPageId, SpkStackedPages::PgQssTest);
#endif
  //============ Sidebar entries END ============

  CategoryWidget->addTopLevelItem(HomepageItem);
  SidebarMgr->AddUnusableItem(CategoryParentItem);
  CategoryWidget->addTopLevelItem(AppDetailsItem);
  CategoryWidget->addTopLevelItem(CategoryParentItem);
  CategoryWidget->addTopLevelItem(DownloadsItem);
  CategoryWidget->addTopLevelItem(UiTestItem);

  CategoryWidget->setFocusPolicy(Qt::NoFocus);

  // Must be done after added into a view.
  AppDetailsItem->setHidden(true); // Hide until we actually open up a Details page
  CategoryParentItem->setExpanded(true);

  // FIXMEIFPOSSIBLE: Fusion adds extra gradient.
  // Details: https://forum.qt.io/topic/128190/fusion-style-kept-adding-an-extra-
  // layer-of-gradient-to-my-selected-item-of-qtreewidget-even-with-qss
  if(SpkUi::OldSystemStyle)
    CategoryWidget->setStyle(SpkUi::OldSystemStyle);
  SidebarMgr->BindCategoryWidget(CategoryWidget);

  HorizontalDivide = new QHBoxLayout;
  HorizontalDivide->setObjectName("spk_mw_divide_hlay");
  HorizontalDivide->setSpacing(0);
  HorizontalDivide->setContentsMargins(0, 0, 0, 0);
  HorizontalDivide->setAlignment(Qt::AlignLeft);
  if(!SpkUi::States::IsUsingDtkPlugin)
    HorizontalDivide->addSpacing(SpkWindow::BorderWidth);
  HorizontalDivide->addWidget(CategoryWidget);
  HorizontalDivide->addWidget(Pager);

  //============ Search Bar ============

  SearchEdit = new SpkFocusLineEdit(this);
  SearchEdit->setPlaceholderText(tr("Press Enter to search"));
  SearchEdit->setFixedWidth(30);
  SearchEdit->setFixedHeight(30);
  SearchBarAnim = new QTimeLine(300, this);
  SearchBarAnim->setDuration(300);
  SearchBarAnim->setEasingCurve(QEasingCurve::OutExpo);
  SearchBarAnim->setUpdateInterval(20);
  ActClearSearchBar = SearchEdit->addAction(QIcon(":/icons/clear-input.svg"),
                                            QLineEdit::TrailingPosition);
  ActClearSearchBar->setVisible(false); // Invisible by default
  ActSearchIcon = SearchEdit->addAction(QIcon(":/icons/search-mini.svg"), QLineEdit::LeadingPosition);
  connect(SearchEdit, &SpkFocusLineEdit::focusGained,
          [=](){
    ActClearSearchBar->setVisible(true);
    SearchBarAnim->setDirection(QTimeLine::Forward);
    SearchBarAnim->start();
  });
  connect(SearchEdit, &SpkFocusLineEdit::focusLost,
             [=](){
    ActClearSearchBar->setVisible(false);
    SearchBarAnim->setDirection(QTimeLine::Backward);
    SearchBarAnim->start();
  });
  connect(SearchBarAnim, &QTimeLine::valueChanged,
          [=](qreal v){
    SearchEdit->setFixedWidth(static_cast<int>(250 * v) + 30);
  });
  connect(ActClearSearchBar, &QAction::triggered, [=](){ SearchEdit->clear(); });
  connect(BtnBack, &QPushButton::clicked,
          [=](){
    SidebarMgr->GoBack();
    BtnBack->setEnabled(false);
  });

  auto space = static_cast<SpkWindow*>(parent)->GetTitleBar()->GetUserSpace();

  space->addSpacing(50);
  space->addWidget(BtnDayNight);
  space->addWidget(BtnSettings);
  space->addWidget(BtnBack);
  space->addWidget(SearchEdit);
  space->addStretch();


  //============ Pages =============

  // Red-Black tree based map will be able to sort things. Just for convenience of ordering pages.
  QMap<SpkStackedPages, QWidget*> sorter;

  // Initialize pages
  PageAppList = new SpkUi::SpkPageAppList(this);
  PageAppList->setProperty("spk_pageid", SpkStackedPages::PgAppList);
  sorter[PgAppList] = PageAppList;

  PageAppDetails = new SpkUi::SpkPageAppDetails(this);
  PageAppDetails->setProperty("spk_pageid", SpkStackedPages::PgAppDetails);
  sorter[PgAppDetails] = PageAppDetails;

  PageDownloads = new SpkUi::SpkPageDownloads(this);
  PageDownloads->setProperty("spk_pageid", SpkStackedPages::PgDownloads);
  sorter[PgDownloads] = PageDownloads;

  PageSettings = new SpkUi::SpkPageSettings(this);
  PageSettings->setProperty("spk_pageid", SpkStackedPages::PgSettings);
  sorter[PgSettings] = PageSettings;

  PageHome = new SpkUi::SpkPageHome(this);
  PageSettings->setProperty("spk_pageid", SpkStackedPages::PgHomepage);
  sorter[PgHomepage] = PageHome;

#ifndef NDEBUG // If only in debug mode should we initialize QSS test page
  PageQssTest = new SpkUi::SpkPageUiTest(this);
  PageQssTest->setProperty("spk_pageid", SpkStackedPages::PgQssTest);
  sorter[PgQssTest] = PageQssTest;
#endif

  for(auto i : sorter)
    Pager->addWidget(i);

  // Default page selection : homepage
  HomepageItem->setSelected(true);

  setLayout(HorizontalDivide);
}
