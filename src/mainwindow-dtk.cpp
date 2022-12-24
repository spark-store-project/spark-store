#include "mainwindow-dtk.h"
#include "ui_mainwindow-dtk.h"

#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <DApplicationSettings>
#include <QSizePolicy>

#define AppPageApplist 0
#define AppPageSearchlist 1
#define AppPageAppdetail 2
#define AppPageSettings 3
#define WaylandSearchCenter 1
#define OtherSearchCenter 2
#define RightSearchSpace 1

MainWindow::MainWindow(QWidget *parent)
    : BaseWidgetOpacity(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initConfig();
    moveToCenter(this); // 让窗口居中显示

    WidgetAnimation::widgetOpacity(this, true);

    downloadlistwidget = new DownloadListWidget;
    downloadButton = new ProgressButton(ui->titlebar);
    backButtom = new QPushButton(ui->titlebar);
    ui->appintopage->setDownloadWidget(downloadlistwidget);
    ui->stackedWidget->setCurrentIndex(0);
    ui->titlebar->setBackgroundTransparent(true);
    // ui->titlebar->setSwitchThemeMenuVisible(false); // 去除 dtk 标题栏主题切换菜单
    setMaskAlpha(250);

    updateUi(0);

    QAction *actionSubmission = new QAction(tr("Submit App"), this);
    QAction *actionSubmissionWithClient = new QAction(tr("Submit App with client(Recommanded)"), this);
    QAction *setting = new QAction(tr("Settings"));
    QAction *upgrade = new QAction(tr("APP Upgrade and Install Settings"));
    QMenu *menu = new QMenu;
    menu->addAction(setting);
    menu->addAction(upgrade);
    menu->addAction(actionSubmission);
    menu->addAction(actionSubmissionWithClient);

    ui->titlebar->setMenu(menu);
    connect(actionSubmission, &QAction::triggered, this, [=]
            { QDesktopServices::openUrl(QUrl("https://upload.deepinos.org/")); });
    connect(setting, &QAction::triggered, this, [=]
            {
        switchPage(AppPageSettings);
        ui->settingspage->updateUI(); });
    connect(upgrade, &QAction::triggered, this, [=]
            { QProcess::startDetached("/opt/durapps/spark-store/bin/update-upgrade/ss-update-controler.sh"); });
    // 投稿器
    connect(actionSubmissionWithClient, &QAction::triggered, this, [=]
            {
                QString submitterSpk = "spk://store/tools/spark-store-submitter";
                QFile actionSubmissionClientStatus("/opt/spark-store-submitter/bin/spark-store-submitter");
                if (actionSubmissionClientStatus.exists())
                {
                    qDebug() << "投稿器存在";
                    QProcess::startDetached("/opt/spark-store-submitter/bin/spark-store-submitter");
                }
                else
                {
                    qDebug() << "投稿器不存在，跳转页面";
                    openUrl(submitterSpk);
                } });

    // 主题切换
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [=](DGuiApplicationHelper::ColorType themeType)
            {
        if (themeType == DGuiApplicationHelper::DarkType) {
            //深色模式
            setMaskColor(QColor("#2a2b2b"));
            this->setStyleSheet("#mainpage{background-color: transparent;border-radius:14px;}\
                                QLabel#cardtitle,QLabel#title,QLabel#title_1,QLabel#title_2,QLabel#title_3 {color:#FFFFFF}\
                                ");
            backButtom->setIcon(QIcon(":/icon/dark/back.svg"));
            downloadButton->setIcon(":/icon/dark/download.svg");
            downloadButton->setBackgroundColor(QColor("#444444"));
            downloadButton->setColor(QColor("#66CCFF"));
            ui->pushButton_14->setIcon(QIcon(":/icon/dark/update.svg"));
            for (int i = 0; i < ui->buttonGroup->buttons().size(); i++) {
                ui->buttonGroup->buttons()[i]->setIcon(QIcon(":/icon/dark/leftbutton_" + QString::number(i) + ".svg"));
                if (QLocale::system().name() == "zh_CN") {
                    ui->buttonGroup->buttons()[i]->setStyleSheet("QPushButton{background-color:transparent;}\
                                                                  QPushButton:hover{background-color:#7a7a7a;border:0px;border-radius:8px;}\
                                                                  QPushButton:checked{background-color:#6e6e6e;border:0px;border-radius:8px;}");
                } else {
                    ui->buttonGroup->buttons()[i]->setStyleSheet("QPushButton{background-color:transparent;text-align: left; padding-left: 15px;}\
                                                                  QPushButton:hover{background-color:#7a7a7a;border:0px;border-radius:8px;text-align: left; padding-left: 15px;}\
                                                                  QPushButton:checked{background-color:#6e6e6e;border:0px;border-radius:8px;text-align: left; padding-left: 15px;}");
                }
            }
        } else {
            //亮色模式
            setMaskColor(QColor("#f3f7f8"));
            this->setStyleSheet("#mainpage{background-color: transparent;border-radius:14px;}\
                                QLabel#cardtitle,QLabel#title,QLabel#title_1,QLabel#title_2,QLabel#title_3 {color:#000000}\
                                ");
            backButtom->setIcon(QIcon(":/icon/light/back.svg"));
            downloadButton->setBackgroundColor(QColor("#e3e4e4"));
            downloadButton->setColor(QColor("#66CCFF"));
            downloadButton->setIcon(":/icon/light/download.svg");
            ui->pushButton_14->setIcon(QIcon(":/icon/light/update.svg"));
            for (int i = 0; i < ui->buttonGroup->buttons().size(); i++) {
                ui->buttonGroup->buttons()[i]->setIcon(QIcon(":/icon/light/leftbutton_" + QString::number(i) + ".svg"));
                if (QLocale::system().name() == "zh_CN") {
                    ui->buttonGroup->buttons()[i]->setStyleSheet("QPushButton{background-color:transparent;}\
                                                                  QPushButton:hover{background-color:#eAeAeA;border:0px;border-radius:8px;}\
                                                                  QPushButton:checked{background-color:#dddddd;border:0px;border-radius:8px;}");
                } else {
                    ui->buttonGroup->buttons()[i]->setStyleSheet("QPushButton{background-color:transparent;text-align: left; padding-left: 15px;}\
                                                                  QPushButton:hover{background-color:#eAeAeA;border:0px;border-radius:8px;text-align: left; padding-left: 15px;}\
                                                                  QPushButton:checked{background-color:#dddddd;border:0px;border-radius:8px;text-align: left; padding-left: 15px;}");
                }
            }
        }
        ui->pushButton_14->setStyleSheet(ui->pushButton_4->styleSheet());
        ui->applistpage->setTheme(themeType == DGuiApplicationHelper::DarkType);
        ui->applistpage_1->setTheme(themeType == DGuiApplicationHelper::DarkType);
        ui->appintopage->setTheme(themeType == DGuiApplicationHelper::DarkType);
        ui->settingspage->setTheme(themeType == DGuiApplicationHelper::DarkType); });

    // 初始化标题栏控件
    connect(downloadButton, &ProgressButton::clicked, [=]()
            {
        QPoint pos;
        pos.setX(downloadButton->mapToGlobal(QPoint(0, 0)).x() + downloadButton->width() / 2 - downloadlistwidget->width() / 2);
        pos.setY(downloadButton->mapToGlobal(QPoint(0, 0)).y() + downloadButton->height() + 5);
        downloadlistwidget->m_move(pos.x(), pos.y());
        downloadlistwidget->show(); });

    // appintopage按下下载按钮时标题栏下载列表按钮抖动
    connect(ui->appintopage, &AppIntoPage::clickedDownloadBtn, [=]()
            {
                WidgetAnimation::widgetShake(downloadButton, 6); // 第一个参数是抖动的控件，第二个参数是抖动范围（像素）
            });

    connect(backButtom, &QPushButton::clicked, [=]()
            {
        ui->stackedWidget->setCurrentIndex(pageHistory.at(pageHistory.count() - 2));
        pageHistory.removeLast();
        if (pageHistory.count() > 1) {
            backButtom->show();
        } else {
            backButtom->hide();
        } });
    downloadlistwidget->hide();
    backButtom->hide();
    ui->titlebar->setIcon(QIcon::fromTheme(":/icon/logo.svg"));

    // Check wayland configs
    QSettings readConfig(QDir::homePath() + "/.config/spark-store/config.ini", QSettings::IniFormat);

    QWidget *w_titlebar = new QWidget(ui->titlebar);
    QHBoxLayout *ly_titlebar = new QHBoxLayout(w_titlebar);
    QLabel *title = new QLabel(this);
    title->setText(tr("Spark Store"));
    searchEdit->setPlaceholderText(tr("Search or enter spk://"));
    ly_titlebar->addWidget(title);
    ly_titlebar->addWidget(backButtom);

    if (!readConfig.value("build/isDeepinOS").toBool() && readConfig.value("build/useWayland").toBool())
    {
        // Wayland 搜索栏居中
        ly_titlebar->addStretch(WaylandSearchCenter);
    }
    else
    {
        // dwayland dxcb 搜索栏顶部右侧居中
        ly_titlebar->addStretch(OtherSearchCenter);
    }

    ly_titlebar->addWidget(searchEdit);
    ly_titlebar->addWidget(downloadButton);
    ly_titlebar->addStretch(RightSearchSpace);
    ui->titlebar->setCustomWidget(w_titlebar);
    // 侧边栏按钮
    int i = 0;
    while (i < ui->buttonGroup->buttons().size())
    {
        ui->buttonGroup->buttons()[i]->setStyleSheet("QPushButton{qproperty-icon: url(data/images/userMangaer/teacher.png);}");
        connect(ui->buttonGroup->buttons()[i], &QPushButton::toggled, [=](bool checked)
                {
            if (checked == true) {
                searchEdit->clearEdit();
                updateUi(i);
            } });
        i++;
    }

    // 搜索事件
    connect(searchEdit, &DSearchEdit::returnPressed, this, [=]()
            {
        QString searchtext = searchEdit->text();
        if (!searchtext.isEmpty()) {
            if (searchtext.startsWith("spk://")) {
                openUrl(QUrl(searchtext));
                searchEdit->clearEdit();
            } else {
                ui->applistpage_1->getSearchList(searchtext);
                switchPage(AppPageSearchlist);
            }
        }
        this->setFocus(); });

    connect(downloadlistwidget, &DownloadListWidget::downloadProgress, this, [=](int i)
            { downloadButton->setProgress(i); });
    // 列表点击事件
    connect(ui->applistpage, &AppListPage::clicked, this, [=](QUrl spk)
            { openUrl(spk); });
    connect(ui->applistpage_1, &AppListPage::clicked, this, [=](QUrl spk)
            { openUrl(spk); });
    connect(ui->settingspage, &SettingsPage::openUrl, this, [=](QUrl spk)
            { openUrl(spk); });
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::instance()->themeType());

    initDbus();
}

MainWindow::~MainWindow()
{
    delete searchEdit;
    delete downloadlistwidget;
    delete ui;
}

void MainWindow::initDbus()
{
    DBusSparkStoreService *dbusInter = new DBusSparkStoreService(this);

    QDBusConnection::sessionBus().registerService("com.gitee.spark.store");
    QDBusConnection::sessionBus().registerObject("/com/gitee/spark/store", "com.gitee.spark.store", this);
    connect(dbusInter, &DBusSparkStoreService::sigOpenUrl, this, &MainWindow::onGetUrl);
}

void MainWindow::onGetUrl(const QString &url)
{
    if (url.left(6) == "spk://")
    {
        openUrl(QUrl(url));
    }
    activateWindow();
}

void MainWindow::openUrl(QUrl url)
{
    if (url.toString().startsWith("spk://"))
    {
        ui->appintopage->openUrl(QUrl(url.toString().replace("+", "%2B")));
        switchPage(AppPageAppdetail);
    }
    else
    {
        QDesktopServices::openUrl(QUrl(url.toString().replace("+", "%2B")));
    }
}

void MainWindow::initConfig()
{
    // 新建临时文件夹
    QDir dir;
    dir.mkpath("/tmp/spark-store");
}

void MainWindow::switchPage(int now) // 临时方案，回家后修改
{
    qDebug() << pageHistory.count();
    if (pageHistory.count() >= 1)
    {
        backButtom->show();
    }
    else
    {
        backButtom->hide();
    }
    ui->stackedWidget->setCurrentIndex(now);
    pageHistory << now;
}

// 刷新界面
void MainWindow::updateUi(int now)
{
    pageHistory.clear();
    QStringList itemlist;
    itemlist << ""
             << "network"
             << "chat"
             << "music"
             << "video"
             << "image_graphics"
             << "games"
             << "office"
             << "reading"
             << "development"
             << "tools"
             << "themes"
             << "others";
    ui->applistpage->getAppList(itemlist[now]);
    qDebug() << itemlist[now];
    switchPage(AppPageApplist);
}

void MainWindow::on_pushButton_14_clicked()
{
    QFile upgradeStatus("/tmp/spark-store/upgradeStatus.txt");
    if (!upgradeStatus.exists())
    {
        QtConcurrent::run([=]
                          {
            auto upgradeP = new QProcess();
            upgradeP->startDetached("/opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade.sh");
            upgradeP->waitForStarted();
            upgradeP->waitForFinished(-1);
            upgradeP->deleteLater(); });
    }
}
