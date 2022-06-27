#include <fstream>
#include <libnotify/notify.h>

#include "widget.h"
#include "ui_widget.h"

#include <QVBoxLayout>
#include <QClipboard>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QProcess>
#include <QSettings>
#include <QGraphicsOpacityEffect>
#include <QtConcurrent> // 并发

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DDialog>
#include <DSettingsDialog>
#include <DSettingsOption>
#include <DStyle>

#include "appitem.h"
#include "flowlayout.h"

#include "HttpClient.h"
#include "downloadworker.h"

DWIDGET_USE_NAMESPACE

Widget::Widget(DBlurEffectWidget *parent) :
    DBlurEffectWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    initUI();
    initConfig();

    manager = new QNetworkAccessManager(this);  // 下载管理

    httpClient = new AeaQt::HttpClient;
    

    connect(ui->menu_main, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(0);});
    connect(ui->menu_network, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(1);});
    connect(ui->menu_chat, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(2);});
    connect(ui->menu_music, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(3);});
    connect(ui->menu_video, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(4);});
    connect(ui->menu_photo, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(5);});
    connect(ui->menu_game, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(6);});
    connect(ui->menu_office, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(7);});
    connect(ui->menu_read, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(8);});
    connect(ui->menu_dev, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(9);});
    connect(ui->menu_system, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(10);});
    connect(ui->menu_theme, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(11);});
    connect(ui->menu_other, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(12);});
    connect(ui->menu_download, &QPushButton::clicked, this, [=](){Widget::chooseLeftMenu(13);});

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [=](DGuiApplicationHelper::ColorType themeType)
    {
        // 获取系统活动色
        main_color = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();

        if(themeType == DGuiApplicationHelper::DarkType)
        {
            qDebug() << "Dark";
            themeIsDark = true;
        }
        else if(themeType == DGuiApplicationHelper::LightType)
        {
            qDebug() << "Light";
            themeIsDark = false;
        }

        // 设置 UI 主题
        setTheme(themeIsDark, main_color);
    });

    connect(&appinfoLoadThread, SIGNAL(requestResetUi()), this, SLOT(sltAppinfoResetUi()), Qt::ConnectionType::BlockingQueuedConnection);
    connect(&appinfoLoadThread, &SpkAppInfoLoaderThread::requestSetTags, this, &Widget::sltAppinfoTags, Qt::ConnectionType::BlockingQueuedConnection);
    connect(&appinfoLoadThread, &SpkAppInfoLoaderThread::requestSetAppInformation, this, &Widget::sltAppinfoDetails, Qt::ConnectionType::BlockingQueuedConnection);
    connect(&appinfoLoadThread, &SpkAppInfoLoaderThread::finishedIconLoad, this, &Widget::sltAppinfoIcon, Qt::ConnectionType::BlockingQueuedConnection);
    connect(&appinfoLoadThread, &SpkAppInfoLoaderThread::finishedScreenshotLoad, this, &Widget::sltAppinfoScreenshot, Qt::ConnectionType::BlockingQueuedConnection);
    connect(&appinfoLoadThread, &SpkAppInfoLoaderThread::finishAllLoading, this, &Widget::sltAppinfoFinish, Qt::ConnectionType::BlockingQueuedConnection);

    // 搜索事件
    connect(searchEdit, &DSearchEdit::returnPressed, this, [=]()
    {
        qDebug() << "触发了搜索，呜啦啦啦!";
        QString searchtext = searchEdit->text();
        if(!searchtext.isEmpty())
        {
            qDebug() << searchEdit->text();
            searchApp(searchtext);
        }
        this->setFocus();
    });

    // 计算显示下载速度
    download_speed.setInterval(1000);
    download_speed.start();
    connect(&download_speed,&QTimer::timeout,[=]()
    {
        if(isdownload)
        {
            size1 = download_size;
            QString theSpeed;
            double bspeed;
            bspeed = size1 - size2;
            if(bspeed < 1024)
            {
                theSpeed = QString::number(bspeed) + "B/s";
            }
            else if(bspeed < 1024 * 1024)
            {
                theSpeed = QString::number(0.01 * int(100 * (bspeed / 1024))) + "KB/s";
            }
            else if(bspeed < 1024 * 1024 * 1024)
            {
                theSpeed = QString::number(0.01 * int(100 * (bspeed / (1024 * 1024)))) + "MB/s";
            }
            else
            {
                theSpeed = QString::number(0.01 * int(100 * (bspeed / (1024 * 1024 * 1024)))) + "GB/s";
            }
            download_list[nowDownload - 1].setSpeed(theSpeed);
            size2 = download_size;
        }
    });

    notify_init(tr("Spark\\ Store").toLocal8Bit());
}

Widget::~Widget()
{
    notify_uninit();

    // delete httpFinished;
    delete ui;

    qDebug() << "exit";
    DApplication::quit();
}

void Widget::initUI()
{
    // ui初始化
    setMaskAlpha(200);
    ui->webfoot->setFixedHeight(0);
    ui->stackedWidget->setCurrentIndex(0);
    ui->listWidget->hide();
    ui->label_setting1->hide();
    ui->pushButton_uninstall->hide();
    ui->line1_widget->setStyleSheet("background-color:#808080");
    ui->icon->setPixmap(QIcon::fromTheme("spark-store").pixmap(36,36));
    ui->titlebar->setFixedHeight(48);

    m_loadweb = ui->progressload;

    label_screen << ui->screen_0 << ui->screen_1 << ui->screen_2 << ui->screen_3 << ui->screen_4;

    // 初始化分界线
    QGraphicsOpacityEffect *opacityEffect_1 = new QGraphicsOpacityEffect;
    opacityEffect_1->setOpacity(0.1);
    ui->line1_widget->setGraphicsEffect(opacityEffect_1);

    // 搜索框
    QWidget *w_titlebar = new QWidget;
    QHBoxLayout *ly_titlebar = new QHBoxLayout;
    w_titlebar->setLayout(ly_titlebar);
    // ly_titlebar->addWidget(ui->pushButton_return);
    ly_titlebar->addStretch();
    ly_titlebar->addSpacing(50);
    ly_titlebar->addWidget(searchEdit);
    ly_titlebar->addStretch();
    titlebar = ui->titlebar;
    titlebar->setCustomWidget(w_titlebar);
    // titlebar->setIcon(QIcon::fromTheme("spark-store"));
    titlebar->setTitle(tr("Spark Store"));
    searchEdit->setPlaceholderText(tr("Search or enter spk://"));
    searchEdit->setFixedWidth(300);
    titlebar->setSeparatorVisible(false);
    // titlebar->setAutoHideOnFullscreen(true);

    // 添加菜单项
    QAction *actionSubmission = new QAction(tr("Submit App"), this);
    QAction *setting = new QAction(tr("Settings"));
    QAction *upgrade = new QAction(tr("APP Upgrade and Install Settings"));

    QMenu *menu = new QMenu;
    menu->addAction(setting);
    menu->addAction(upgrade);
    menu->addAction(actionSubmission);

    titlebar->setMenu(menu);

    connect(actionSubmission, &QAction::triggered, this, [=]{QDesktopServices::openUrl(QUrl("https://upload.deepinos.org/"));});
    connect(setting, &QAction::triggered, this, &Widget::opensetting);
    connect(upgrade, &QAction::triggered, this, [=]{QProcess::startDetached("x-terminal-emulator -e /opt/durapps/spark-store/bin/update-upgrade/ss-update-controler.sh");});

    // 载入自定义字体
    int loadedFontID = QFontDatabase::addApplicationFont(":/fonts/fonts/hksnzt.ttf");
    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
    if(!loadedFontFamilies.isEmpty())
    {
        font = loadedFontFamilies.at(0);
    }
    /* 全局字体设置
     * DApplication::setFont(font);
     */

    // 初始化菜单数组
    left_list[0] = ui->menu_main;
    left_list[1] = ui->menu_network;
    left_list[2] = ui->menu_chat;
    left_list[3] = ui->menu_music;
    left_list[4] = ui->menu_video;
    left_list[5] = ui->menu_photo;
    left_list[6] = ui->menu_game;
    left_list[7] = ui->menu_office;
    left_list[8] = ui->menu_read;
    left_list[9] = ui->menu_dev;
    left_list[10] = ui->menu_system;
    left_list[11] = ui->menu_theme;
    left_list[12] = ui->menu_other;
    left_list[13] = ui->menu_download;

    ui->label_show->hide();

    // 搜索列表页
    applist_grid = new FlowLayout;
    main = new QHBoxLayout;
    main->setAlignment(Qt::AlignHCenter);
    main->setMargin(0);
    main->addWidget(spinner);
    ui->applist_scrollAreaWidget->setLayout(main);
    spinner->setFixedSize(80, 80);

    // 初始化系统活动色和主题颜色
    main_color = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
    if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
    {
        themeIsDark = true;
    }
    else
    {
        themeIsDark = false;
    }
    setTheme(themeIsDark, main_color);
}

void Widget::initConfig()
{
    // 读取服务器列表并初始化
    std::fstream serverList;
    serverList.open(QDir::homePath().toUtf8() + "/.config/spark-store/server.list", std::ios::in);
    std::string lineTmp;
    if(serverList)
    {
        while(getline(serverList, lineTmp))
        {
            ui->comboBox_server->addItem(QString::fromStdString(lineTmp));
        }
        for(int i = 0; i < ui->comboBox_server->count(); i++)
        {
            if(ui->comboBox_server->itemText(i) == "开发者模式 Dev only")
            {
                ui->comboBox_server->model()->setData(ui->comboBox_server->model()->index(i, 0), QVariant(0), Qt::UserRole - 1);
            }
            if(ui->comboBox_server->itemText(i) == "镜像源 Download only")
            {
                for(int j = i; j < ui->comboBox_server->count(); j++)
                {
                    ui->comboBox_server->model()->setData(ui->comboBox_server->model()->index(j, 0), QVariant(0), Qt::UserRole - 1);
                }
                break;
            }
        }
    }
    else
    {
        ui->comboBox_server->addItem("https://d.store.deepinos.org.cn/");
    }

    // 读取服务器URL并初始化菜单项的链接
    QSettings readConfig(QDir::homePath() + "/.config/spark-store/config.ini", QSettings::IniFormat);
    if(!readConfig.value("server/choose").toString().isEmpty())
    {
        ui->comboBox_server->setCurrentText(readConfig.value("server/choose").toString());
        appinfoLoadThread.setServer(serverUrl = readConfig.value("server/choose").toString());
    }
    else
    {
        appinfoLoadThread.setServer(serverUrl = "https://d.store.deepinos.org.cn/");  // 默认URL
    }
    configCanSave = true;   //　防止触发保存配置信号

    // menuUrl[0] = "http://127.0.0.1:8000/#/darkprogramming";
    menuUrl[0] = serverUrl + "store/#/";
    menuUrl[1] = serverUrl + "store/#/network";
    menuUrl[2] = serverUrl + "store/#/relations";
    menuUrl[3] = serverUrl + "store/#/musicandsound";
    menuUrl[4] = serverUrl + "store/#/videos";
    menuUrl[5] = serverUrl + "store/#/photos";
    menuUrl[6] = serverUrl + "store/#/games";
    menuUrl[7] = serverUrl + "store/#/office";
    menuUrl[8] = serverUrl + "store/#/reading";
    menuUrl[9] = serverUrl + "store/#/programming";
    menuUrl[10] = serverUrl + "store/#/tools";
    menuUrl[11] = serverUrl + "store/#/themes";
    menuUrl[12] = serverUrl + "store/#/others";

    // web控件初始化
    // ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);    // 用来激活接受 linkClicked 信号
    // ui->webView->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    ui->webfoot->hide();

    // 初始化首页
    // ui->webEngineView->setUrl(menuUrl[0]);
    chooseLeftMenu(0);

    // 给下载列表赋值到数组，方便调用
    for(int i = 0; i < LIST_MAX; i++)
    {
        download_list[i].num = i;
    }

    // 初始化 apt 源显示
    QFile aptserver("/etc/apt/sources.list.d/sparkstore.list");
    aptserver.open(QIODevice::ReadOnly);
    if(aptserver.isOpen())
    {
        ui->label_aptserver->setText(aptserver.readAll());
    }
    else
    {
        ui->label_aptserver->setText(tr("Not Exist"));
    }
    aptserver.close();

    // 新建临时文件夹
    QDir dir("/tmp");
    dir.mkdir("spark-store");
}

void Widget::setTheme(bool isDark, QColor color)
{
    // 菜单图标
    if(isDark)
    {
        // 黑色模式
        themeIsDark = true;
        ui->webEngineView->setStyleSheet("background-color: #252525;");
        ui->webfoot->setStyleSheet("background-color: #252525;");
        ui->btn_openDir->setStyleSheet("color: #AFAFAF; background-color: #2C2C2C; border: 0px;");
        ui->label->setStyleSheet("background-color: #252525;");
        // ui->scrollArea->setStyleSheet("background-color: #2C2C2C;");
        ui->label_show->setStyleSheet("background-color: #2C2C2C;");
        ui->pushButton_return->setIcon(DStyle().standardIcon(DStyle::SP_ArrowLeft));
        ui->pushButton_refresh->setIcon(QIcon(":/icons/icons/refresh-page-dark.svg"));
    }
    else
    {
        // 亮色模式
        themeIsDark = false;
        ui->webEngineView->setStyleSheet("background-color: #FFFFFF;");
        ui->webfoot->setStyleSheet("background-color: #FFFFFF;");
        ui->btn_openDir->setStyleSheet("color: #505050; background-color: #F8F8F8; border: 0px;");
        ui->label->setStyleSheet("background-color: #FFFFFF;");
        // ui->scrollArea->setStyleSheet("background-color: #F8F8F8;");
        ui->label_show->setStyleSheet("background-color: #F8F8F8;");
        ui->pushButton_return->setIcon(DStyle().standardIcon(DStyle::SP_ArrowLeft));
        ui->pushButton_refresh->setIcon(QIcon(":/icons/icons/refresh-page.svg"));
    }

    // 刷新系统活动色
    main_color = color;
    m_loadweb->setTheme(themeIsDark, color);
    updateUI();

    // 刷新网页主题颜色
    if(ui->stackedWidget->currentIndex() == 0)
    {
        chooseLeftMenu(nowMenu);
    }
}

DTitlebar* Widget::getTitlebar()
{
    return ui->titlebar;
}

static NotifyNotification *_notify = nullptr;

void Widget::sendNotification(const QString &message, const int msTimeout, const QString &icon)
{
    if(_notify == nullptr)
    {
        _notify = notify_notification_new(tr("Spark\\ Store").toLocal8Bit(), message.toLocal8Bit(), icon.toLocal8Bit());
        notify_notification_set_timeout(_notify, msTimeout);
    }
    else
    {
        notify_notification_update(_notify, tr("Spark\\ Store").toLocal8Bit(), message.toLocal8Bit(), icon.toLocal8Bit());
    }

    notify_notification_show(_notify, nullptr);
}

void Widget::sendNotification(const char *message, const int msTimeout, const QString &icon)
{
    if(_notify == nullptr)
    {
        _notify = notify_notification_new(tr("Spark\\ Store").toLocal8Bit(), message, icon.toLocal8Bit());
        notify_notification_set_timeout(_notify, msTimeout);
    }
    else
    {
        notify_notification_update(_notify, tr("Spark\\ Store").toLocal8Bit(), message, icon.toLocal8Bit());
    }

    notify_notification_show(_notify, nullptr);
}

void Widget::updateUI()
{
    if(themeIsDark)
    {
        left_list[0]->setIcon(QIcon(":/icons/icons/homepage_dark.svg"));
        left_list[1]->setIcon(QIcon(":/icons/icons/category_network_dark.svg"));
        left_list[2]->setIcon(QIcon(":/icons/icons/category_chat_dark.svg"));
        left_list[3]->setIcon(QIcon(":/icons/icons/category_music_dark.svg"));
        left_list[4]->setIcon(QIcon(":/icons/icons/category_video_dark.svg"));
        left_list[5]->setIcon(QIcon(":/icons/icons/category_graphic_dark.svg"));
        left_list[6]->setIcon(QIcon(":/icons/icons/category_game_dark.svg"));
        left_list[7]->setIcon(QIcon(":/icons/icons/category_office_dark.svg"));
        left_list[8]->setIcon(QIcon(":/icons/icons/category_reading_dark.svg"));
        left_list[9]->setIcon(QIcon(":/icons/icons/category_develop_dark.svg"));
        left_list[10]->setIcon(QIcon(":/icons/icons/category_system_dark.svg"));
        left_list[11]->setIcon(QIcon(":/icons/icons/theme-symbolic_dark.svg"));
        left_list[12]->setIcon(QIcon(":/icons/icons/category_others_dark.svg"));
        left_list[13]->setIcon(QIcon(":/icons/icons/downloads-symbolic_dark.svg"));
    }
    else
    {
        left_list[0]->setIcon(QIcon(":/icons/icons/homepage.svg"));
        left_list[1]->setIcon(QIcon(":/icons/icons/category_network.svg"));
        left_list[2]->setIcon(QIcon(":/icons/icons/category_chat.svg"));
        left_list[3]->setIcon(QIcon(":/icons/icons/category_music.svg"));
        left_list[4]->setIcon(QIcon(":/icons/icons/category_video.svg"));
        left_list[5]->setIcon(QIcon(":/icons/icons/category_graphic.svg"));
        left_list[6]->setIcon(QIcon(":/icons/icons/category_game.svg"));
        left_list[7]->setIcon(QIcon(":/icons/icons/category_office.svg"));
        left_list[8]->setIcon(QIcon(":/icons/icons/category_reading.svg"));
        left_list[9]->setIcon(QIcon(":/icons/icons/category_develop.svg"));
        left_list[10]->setIcon(QIcon(":/icons/icons/category_system.svg"));
        left_list[11]->setIcon(QIcon(":/icons/icons/theme-symbolic.svg"));
        left_list[12]->setIcon(QIcon(":/icons/icons/category_others.svg"));
        left_list[13]->setIcon(QIcon(":/icons/icons/downloads-symbolic.svg"));
    }

    for(int i = 0; i < 14; i++)
    {
        /* 设置左侧菜单字体
         * QFont temp = font;
         * temp.setPixelSize(15);
         * left_list[i]->setFont(temp);
         */

        left_list[i]->setFixedHeight(38);
        if(themeIsDark)
        {
            // 中文环境菜单文字居中，其他则左对齐
            if(QLocale::system().name() == "zh_CN")
            {
                left_list[i]->setStyleSheet("color: #FFFFFF; border: 0px;");
            }
            else
            {
                left_list[i]->setStyleSheet("color: #FFFFFF; border: 0px; text-align: left; padding-left: 15px;");
            }
        }
        else
        {
            if(QLocale::system().name() == "zh_CN")
            {
                left_list[i]->setStyleSheet("color: #252525; border: 0px;");
            }
            else
            {
                left_list[i]->setStyleSheet("color: #252525; border: 0px; text-align: left; padding-left: 15px;");
            }
        }
    }

    if(QLocale::system().name() == "zh_CN")
    {
        left_list[nowMenu]->setStyleSheet("color: #FFFFFF; background-color: " + main_color.name() + "; border-radius: 8; border: 0px;");
    }
    else
    {
        left_list[nowMenu]->setStyleSheet("color: #FFFFFF; background-color: " + main_color.name() + "; border-radius: 8; border: 0px; text-align: left; padding-left: 15px;");
    }

    switch (nowMenu)
    {
    case 0:
        left_list[0]->setIcon(QIcon(":/icons/icons/homepage_dark.svg"));
        break;
    case 1:
        left_list[1]->setIcon(QIcon(":/icons/icons/category_network_dark.svg"));
        break;
    case 2:
        left_list[2]->setIcon(QIcon(":/icons/icons/category_chat_dark.svg"));
        break;
    case 3:
        left_list[3]->setIcon(QIcon(":/icons/icons/category_music_dark.svg"));
        break;
    case 4:
        left_list[4]->setIcon(QIcon(":/icons/icons/category_video_dark.svg"));
        break;
    case 5:
        left_list[5]->setIcon(QIcon(":/icons/icons/category_graphic_dark.svg"));
        break;
    case 6:
        left_list[6]->setIcon(QIcon(":/icons/icons/category_game_dark.svg"));
        break;
    case 7:
        left_list[7]->setIcon(QIcon(":/icons/icons/category_office_dark.svg"));
        break;
    case 8:
        left_list[8]->setIcon(QIcon(":/icons/icons/category_reading_dark.svg"));
        break;
    case 9:
        left_list[9]->setIcon(QIcon(":/icons/icons/category_develop_dark.svg"));
        break;
    case 10:
        left_list[10]->setIcon(QIcon(":/icons/icons/category_system_dark.svg"));
        break;
    case 11:
        left_list[11]->setIcon(QIcon(":/icons/icons/theme-symbolic_dark.svg"));
        break;
    case 12:
        left_list[12]->setIcon(QIcon(":/icons/icons/category_others_dark.svg"));
        break;
    case 13:
        left_list[13]->setIcon(QIcon(":/icons/icons/downloads-symbolic_dark.svg"));
        break;
    }
}

// 菜单切换逻辑
void Widget::chooseLeftMenu(int index)
{
    nowMenu = index;

    // 菜单切换时，清除搜索栏的内容
    if(!searchEdit->text().isEmpty())
    {
        searchEdit->clear();
    }

    updateUI();

    if(index <= 12)
    {
        if(themeIsDark)
        {
            QString darkurl = menuUrl[index].toString();
            QStringList list = darkurl.split("/");
            darkurl.clear();
            for(int i = 0; i < list.size() - 1; i++)
            {
                darkurl += list[i] + "/";
            }
            darkurl += "dark" + list[list.size() - 1];
            ui->webEngineView->setUrl(darkurl);
            qDebug() << darkurl;
        }
        else
        {
            ui->webEngineView->setUrl(menuUrl[index]);
        }

        ui->stackedWidget->setCurrentIndex(0);
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void Widget::setfoot(int h)
{
    foot = h;
}

void Widget::updatefoot()
{
    int allh = ui->stackedWidget->height();
    ui->webfoot->setFixedHeight(allh - foot);
}

void Widget::on_pushButton_download_clicked()
{
    chooseLeftMenu(13);

    allDownload += 1;

    QFileInfo info(url.path());
    QString fileName(info.fileName());  // 获取文件名
    download_list[allDownload - 1].pkgName = pkgName;
    if(fileName.isEmpty())
    {
        sendNotification(tr("Failed to get the name to the file to be downloaded."));
        return;
    }

    download_list[allDownload - 1].setParent(ui->listWidget);
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(download_list[allDownload - 1].size());
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
    ui->listWidget->setItemWidget(item, &download_list[allDownload - 1]);
    urList.append(url);
    download_list[allDownload - 1].setName(appName);
    download_list[allDownload - 1].setFileName(fileName);

    QPixmap icon;
    icon.load("/tmp/spark-store/icon.png", "PNG");
    system("cp /tmp/spark-store/icon.png /tmp/spark-store/icon_" + QString::number(allDownload - 1).toUtf8() + ".png");
    download_list[allDownload - 1].seticon(icon);

    if(!isBusy)
    {
        /*
        file = new QFile(fileName);
        if(!file->open(QIODevice::WriteOnly))
        {
            delete file;
            file = nullptr;
            return;
        }
        */

        nowDownload += 1;
        startRequest(urList.at(nowDownload - 1), fileName);     // 进行链接请求
    }

    if(ui->pushButton_download->text() == tr("Reinstall"))
    {
        download_list[allDownload - 1].reinstall = true;
    }
}

void Widget::startRequest(QUrl url, QString fileName)
{
    ui->listWidget->show();
    ui->label->hide();
    isBusy = true;
    isdownload = true;
    download_list[allDownload - 1].free = false;

    downloadController = new DownloadController(this);  // 并发下载，在点击下载按钮的时候才会初始化
    connect(downloadController, &DownloadController::downloadProcess, this, &Widget::updateDataReadProgress);
    connect(downloadController, &DownloadController::downloadFinished, this, &Widget::httpFinished);
    connect(downloadController, &DownloadController::errorOccur, this, [=](QString msg){this->sendNotification(msg);});
    downloadController->setFilename(fileName);
    downloadController->startDownload(url.toString());
}

void Widget::searchApp(QString text)
{
    if(text.left(6) == "spk://")
    {
        openUrl(text);
        searchEdit->clearEdit();
    }
    else
    {
        // sendNotification(tr("Spark store could only process spk:// links for now. The search feature is coming soon!"));
        // ui->webView->setUrl(QUrl("http://www.baidu.com/s?wd="+text));    // 这东西对接百度
        // ui->stackedWidget->setCurrentIndex(0);

        // 禁止同时进行多次搜索
        if(!mutex.tryLock())
        {
            return;
        }

        // 关键字搜索处理
        httpClient->get("https://search.deepinos.org.cn/appinfo/search")
                .header("content-type", "application/json")
                .queryParam("keyword", text)
                .onResponse([this](QByteArray result)
        {
            auto json = QJsonDocument::fromJson(result).array();
            if (json.empty())
            {
                qDebug() << "相关应用未找到！";
                sendNotification(tr("Relative apps Not Found!"));
                mutex.unlock();
                return;
            }
            displaySearchApp(json);
        })
        .onError([this](QString errorStr)
        {
            qDebug()  << "请求出错：" << errorStr;
            sendNotification(QString(tr("Request Error: %1")).arg(errorStr));
            mutex.unlock();
            return;
        })
        .timeout(10 * 1000)
                .exec();
    }
}

/**
 * @brief 展示搜索的APP信息
 */
void Widget::displaySearchApp(QJsonArray array)
{
    ui->stackedWidget->setCurrentIndex(4);

    // 清除原有的搜索结果
    QLayoutItem *item = nullptr;
    while((item = applist_grid->takeAt(0)) != nullptr)
    {
        applist_grid->removeWidget(item->widget());
        item->widget()->disconnect();
        item->widget()->setParent(nullptr);
        delete item;
    }

    main->removeItem(applist_grid);
    spinner->show();
    spinner->start();

    for(int i = 0; i < array.size(); i++)
    {
        QJsonObject appInfo = array.at(i).toObject();
        AppItem *appItem = new AppItem(this);
        QString url = QString("spk://store/%1/%2")
                .arg(appInfo["category_slug"].toString())
                .arg(appInfo["pkgname"].toString());
        appItem->setTitle(appInfo["name"].toString());
        appItem->setDescription(appInfo["more"].toString());
        appItem->setIcon(appInfo["icon"].toString());
        appItem->setUrl(url);
        applist_grid->addWidget(appItem);
        qDebug() << "应用链接为：" << url;

        connect(appItem, &AppItem::clicked, this, &Widget::openUrl);
        connect(appItem, &AppItem::clicked, this, [=](){prePage = ui->stackedWidget->currentIndex();});
        connect(appItem, &AppItem::finished, this, [=]()
        {
            count++;
            downloadIconsFinished(array.size());
        });
    }
}

void Widget::downloadIconsFinished(int arraysize)
{
    // 当前搜索列表图标全部加载完成后才允许下一次搜索
    if(count == arraysize)
    {
        spinner->stop();
        spinner->hide();
        main->addLayout(applist_grid, 1);
        count = 0;
        mutex.unlock();
    }
}

void Widget::httpReadyRead()
{
    if(file)
    {
        file->write(reply->readAll());
    }
}

void Widget::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if(totalBytes <= 0)
    {
        return;
    }

    download_list[nowDownload - 1].setMax(10000);   // 最大值
    download_list[nowDownload - 1].setValue((bytesRead * 10000) / totalBytes);  // 当前值
    download_size = bytesRead;
    if(download_list[nowDownload - 1].close)
    {
        // 随时检测下载是否被取消
        downloadController->disconnect();
        downloadController->stopDownload();
        download_list[nowDownload - 1].closeDownload();
        httpFinished();
    }
}

void Widget::sltAppinfoResetUi()
{
    // 先隐藏详情页负责显示截图的 label
    ui->screen_0->hide();
    ui->screen_1->hide();
    ui->screen_2->hide();
    ui->screen_3->hide();
    ui->screen_4->hide();
    ui->label_appicon->clear();
    ui->tag_community->hide();
    ui->tag_ubuntu->hide();
    ui->tag_deepin->hide();
    ui->tag_uos->hide();
    ui->tag_dtk5->hide();
    ui->tag_dwine2->hide();
    ui->tag_dwine5->hide();
    ui->tag_a2d->hide();

    //　重置 UI 状态
    ui->pushButton_uninstall->hide();
    ui->pushButton_website->setEnabled(false);
    ui->pushButton->setEnabled(false);
    ui->pushButton_update->setEnabled(false);
    ui->label_show->setText("Loading...");
    ui->label_show->show();
    ui->label_downloadCount->clear();
}

void Widget::sltAppinfoTags(QStringList *tagList)
{
    foreach(const QString &tag, *tagList)
    {
        if(tag=="community")
        {
            ui->tag_community->show();
        }
        else if(tag=="ubuntu")
        {
            ui->tag_ubuntu->show();
        }
        else if(tag=="deepin")
        {
            ui->tag_deepin->show();
        }
        else if(tag=="uos")
        {
            ui->tag_uos->show();
        }
        else if(tag=="dtk5")
        {
            ui->tag_dtk5->show();
        }
        else if(tag=="dwine2")
        {
            ui->tag_dwine2->show();
        }
        else if(tag=="dwine5")
        {
            ui->tag_dwine5->show();
        }
        else if(tag=="a2d")
        {
            ui->tag_a2d->show();
        }
    }
}

void Widget::sltAppinfoDetails(QString *name, QString *details, QString *info,
                               QString *website, QString *packageName, QUrl *fileUrl,
                               int downloadCount, bool isInstalled, bool isUpdated)
{
    ui->label_appname->setText(appName = *name);
    ui->label_appname->show();
    ui->label_info->setText(*details);
    ui->label_info->show();
    ui->label_more->setText(*info);
    ui->label_more->show();
    ui->label_downloadCount->setText(tr("Total downloads: %1").arg(downloadCount));

    pkgName = *packageName;
    url = *fileUrl;
    appweb = *website;

    if(isInstalled)
    {
        if(isUpdated)
        {
            ui->pushButton_download->setText(tr("Reinstall"));
            ui->pushButton_uninstall->show();
        }
        else
        {
            ui->pushButton_download->setText(tr("Upgrade"));
            ui->pushButton_uninstall->show();
        }
    }
    else
    {
        ui->pushButton_download->setText(tr("Install"));
    }
}

void Widget::sltAppinfoIcon(QPixmap *icon)
{
    ui->label_appicon->setPixmap(*icon);
    ui->label_appicon->show();
    ui->pushButton_download->setEnabled(true);
    ui->pushButton->setEnabled(true);
    ui->pushButton_update->setEnabled(true);
    ui->pushButton_website->setEnabled(true);
}

void Widget::sltAppinfoScreenshot(QPixmap *picture, int index)
{
    if(picture != nullptr)
    {
        screen[index] = *picture;
        label_screen[index]->setImage(*picture);
        label_screen[index]->show();

        switch(index)
        {
        // 故意为之，为了清除多余截图
        case 0:
            label_screen[1]->hide();
        case 1:
            label_screen[2]->hide();
        case 2:
            label_screen[3]->hide();
        case 3:
            label_screen[4]->hide();
        }
    }
}

void Widget::sltAppinfoFinish()
{
    ui->label_show->clear();
    ui->label_show->hide();
}

void Widget::httpFinished() // 完成下载
{
    isdownload = false;
    isBusy = false;
    download_list[nowDownload - 1].readyInstall();
    download_list[nowDownload - 1].free = true;
    if(nowDownload < allDownload)
    {
        // 如果有排队则下载下一个
        nowDownload += 1;
        while(download_list[nowDownload - 1].close)
        {
            nowDownload += 1;
        }
        QString fileName = download_list[nowDownload - 1].getName();
        startRequest(urList.at(nowDownload-1), fileName);
    }
}

void Widget::on_pushButton_return_clicked()
{
    appinfoLoadThread.requestInterruption();

    // 检测是否是从搜索页面进入到应用详情页的，根据搜索栏是否有关键词判断
    if (searchEdit->text().isEmpty())
    {
        if(ui->stackedWidget->currentIndex() == 2)
        {
            ui->webEngineView->back();
            ui->stackedWidget->setCurrentIndex(0);
        }
        else
        {
            ui->stackedWidget->setCurrentIndex(prePage);
        }
    }
    else
    {
        if (ui->stackedWidget->currentIndex() == 4)
        {
            searchEdit->clear();
            chooseLeftMenu(nowMenu);
        }
        else if(ui->stackedWidget->currentIndex() == 2)
        {
            ui->stackedWidget->setCurrentIndex(4);
        }
        else
        {
            ui->stackedWidget->setCurrentIndex(prePage);
        }

        if(ui->stackedWidget->currentIndex() == 4)
        {
            searchApp(searchEdit->text());
        }
    }
}

void Widget::on_pushButton_refresh_clicked()
{
    // 如果在详情页面要重新触发 UrlChanged
    if(ui->stackedWidget->currentIndex() == 2)
    {
        emit ui->webEngineView->urlChanged(ui->webEngineView->url());
    }
    else
    {
        ui->webEngineView->reload();
    }
}

void Widget::on_comboBox_server_currentIndexChanged(const QString &arg1)
{
    appinfoLoadThread.setServer(arg1);  // 服务器信息更新

    if(configCanSave)
    {
        // ui->label_setting1->show();
        QSettings *setConfig = new QSettings(QDir::homePath() + "/.config/spark-store/config.ini", QSettings::IniFormat);
        setConfig->setValue("server/choose", arg1);
    }
}

void Widget::on_pushButton_updateServer_clicked()
{
    QtConcurrent::run([=]()
    {
        ui->pushButton_updateServer->setEnabled(false);
        ui->comboBox_server->clear();

        QFile::remove(QDir::homePath().toUtf8() + "/.config/spark-store/server.list");
        system("curl -o " + QDir::homePath().toUtf8() + "/.config/spark-store/server.list https://d.store.deepinos.org.cn/store/server.list");
        std::fstream server;
        server.open(QDir::homePath().toUtf8() + "/.config/spark-store/server.list", std::ios::in);
        std::string lineTmp;
        if(server)
        {
            while(getline(server, lineTmp))
            {
                ui->comboBox_server->addItem(QString::fromStdString(lineTmp));
            }
        }
        else
        {
            ui->comboBox_server->addItem("https://d.store.deepinos.org.cn/");
        }

        ui->pushButton_updateServer->setEnabled(true);
        ui->comboBox_server->setCurrentIndex(0);

        for(int i = 0; i < ui->comboBox_server->count(); i++)
        {
            if(ui->comboBox_server->itemText(i) == "开发者模式 Dev only")
            {
                ui->comboBox_server->model()->setData(ui->comboBox_server->model()->index(i, 0), QVariant(0), Qt::UserRole - 1);
            }
            if(ui->comboBox_server->itemText(i) == "镜像源 Download only")
            {
                for(int j = i; j < ui->comboBox_server->count(); j++)
                {
                    ui->comboBox_server->model()->setData(ui->comboBox_server->model()->index(j, 0), QVariant(0), Qt::UserRole - 1);
                }
                break;
            }
        }
    });
}

void Widget::on_pushButton_updateApt_clicked()
{
    QtConcurrent::run([=]()
    {
        ui->pushButton_updateApt->setEnabled(false);
        ui->label_aptserver->setText(tr("Updating, please wait..."));

        std::fstream sourcesList, policy, update;
        QDir tmpdir("/tmp");
        auto tmpPath = QString::fromUtf8(TMP_PATH).toStdString();
        bool unknownError = true;

        tmpdir.mkpath("spark-store");
        sourcesList.open(tmpPath + "/sparkstore.list", std::ios::out);
        // 商店已经下架会替换系统库的包，优先级 policy 弃用
        // policy.open(tmpPath + "/sparkstore", std::ios::out);

        if(sourcesList /*&& policy*/)
        {
            auto serverAddr = ui->comboBox_server->currentText();

            sourcesList << "deb [by-hash=force] ";
            sourcesList << serverAddr.toUtf8().toStdString();
            sourcesList << " /";
            sourcesList.close();

            /*
             * policy << "Package: *\n" << "Pin: origin *" <<
             * serverAddr.mid(serverAddr.indexOf('.')).toUtf8().toStdString() << "\n" <<
             * "Pin-Priority: 90"; // 降低星火源的优先级，防止从星火安装已存在的系统包，破坏依赖
             * policy.close();
           */

            update.open(tmpPath + "/update.sh", std::ios::out);
            if(update)
            {
                unknownError = false;
                update << "#!/bin/sh\n" <<
                          "mv " + tmpPath + "/sparkstore.list /etc/apt/sources.list.d/sparkstore.list && " <<
                          // "mv " + tmpPath + "/sparkstore /etc/apt/preferences.d/sparkstore && " <<
                          "apt update";
                update.close();

                system(("chmod +x " + tmpPath + "/update.sh").c_str());

                QProcess runupdate;
                runupdate.start("pkexec", QStringList() << QString::fromStdString(tmpPath + "/update.sh"));
                runupdate.waitForFinished();
                QString error = runupdate.readAllStandardError();

                QStringList everyError = error.split("\n");
                bool haveError = false;
                for(int i = 0; i < everyError.size(); i++)
                {
                    if(everyError[i].left(2) == "E:")
                    {
                        haveError = true;
                    }
                }

                if(!haveError)
                {
                    ui->label_aptserver->setText("deb [by-hash=force] " + ui->comboBox_server->currentText().toUtf8() + " /");
                }
                else
                {
                    ui->label_aptserver->setText(tr("Apt has reported an error. Please use apt update in terminal to locate the problem."));
                }
            }
        }

        if(unknownError)
        {
            ui->label_aptserver->setText(tr("Unknown error!"));
        }

        ui->pushButton_updateApt->setEnabled(true);
    });
}

void Widget::on_pushButton_uninstall_clicked()
{
    QtConcurrent::run([=]()
    {
        ui->pushButton_download->setEnabled(false);
        ui->pushButton_uninstall->setEnabled(false);

        QProcess uninstall;
        uninstall.start("pkexec", QStringList() << "apt" << "purge" << "-y" << pkgName.toLower());
        uninstall.waitForFinished();

        QProcess check;
        check.start("dpkg", QStringList() << "-s" << pkgName.toLower());
        check.waitForFinished();

        if (check.readAllStandardOutput().isEmpty())
        {
            ui->pushButton_download->setText(tr("Install"));
            ui->pushButton_uninstall->hide();

            updatesEnabled();
            sendNotification(tr("Uninstall succeeded"));
        }

        ui->pushButton_download->setEnabled(true);
        ui->pushButton_uninstall->setEnabled(true);
    });
}

void Widget::on_pushButton_clear_clicked()  // 清空临时缓存目录
{
    QtConcurrent::run([=]()
    {
        ui->pushButton_clear->setEnabled(false);

        QDir tmpdir("/tmp/spark-store");
        tmpdir.setFilter(QDir::Files);
        int quantity = int(tmpdir.count());
        for(int i = 0; i < quantity; i++)
        {
            tmpdir.remove(tmpdir[i]);
        }

        sendNotification(tr("Temporary cache was cleaned"));
        ui->pushButton_clear->setEnabled(true);
        Widget::opensetting();
    });
}

quint64 Widget::dirFileSize(const QString &path)
{
    QDir dir(path);
    quint64 size = 0;
    // dir.entryInfoList(QDir::Files);  // 返回文件信息
    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Files))
    {
        // 计算文件大小
        size += quint64(fileInfo.size());
    }
    // dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);  // 返回所有子目录，并进行过滤
    foreach(QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        // 若存在子目录，则递归调用 dirFileSize() 函数
        size += dirFileSize(path + QDir::separator() + subDir);
    }
    return size;
}

void Widget::opensetting()
{
    // 防止下载时文件被删除
    if(isdownload)
    {
        ui->pushButton_clear->setEnabled(false);
    }
    else
    {
        ui->pushButton_clear->setEnabled(true);
    }

    // 显示缓存占用空间
    quint64 tmp_size = dirFileSize(QString::fromUtf8(TMP_PATH));
    QString tmp_size_str;
    if(tmp_size < 1024)
    {
        tmp_size_str = QString::number(tmp_size) + "B";
    }
    else if(tmp_size < (1024 * 1024))
    {
        tmp_size_str = QString::number(0.01 * int(100 * (tmp_size / 1024))) + "KB";
    }
    else if(tmp_size<(1024*1024*1024))
    {
        tmp_size_str = QString::number(0.01 * int(100 * (tmp_size / (1024 * 1024)))) + "MB";
    }
    else
    {
        tmp_size_str = QString::number(0.01 * int(100 * (tmp_size / (1024 * 1024 * 1024)))) + "GB";
    }

    ui->tmp_size_ui->setText(tmp_size_str);
    prePage = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(3);
}

void Widget::openUrl(QUrl url)
{
    QString app = serverUrl + "store" + url.path() + "/app.json";
    // ui->webEngineView->setUrl(app);
    emit ui->webEngineView->urlChanged(app);
}

void Widget::on_pushButton_website_clicked()
{
    QDesktopServices::openUrl(QUrl(appweb));
}

void Widget::on_pushButton_clicked()
{
    QString share_url;
    share_url = "spk://store/" + type_name + "/" + pkgName;
    qDebug() << "Share" << share_url;

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(share_url);
    sendNotification(tr("The URL has been copied to the clipboard"));
}

void Widget::on_btn_openDir_clicked()
{
    QDesktopServices::openUrl(QUrl("file:///tmp/spark-store", QUrl::TolerantMode));
}

void Widget::on_stackedWidget_currentChanged(int arg1)
{
    qDebug() << arg1;
    if(arg1 == 0 || arg1 == 1)
    {
        ui->pushButton_return->setEnabled(false);
    }
    else
    {
        ui->pushButton_return->setEnabled(true);
    }
}

void Widget::on_webEngineView_urlChanged(const QUrl &arg1)
{
    //分析出服务器中的分类名称
    QStringList url_ = arg1.path().split("/");
    QString pname;
    if(url_.size() > 3)
    {
        type_name = url_[2];
        pname = url_[3];
    }
    //如果是app.json就打开详情页
    if(arg1.path().right(8) == "app.json")
    {
        prePage = ui->stackedWidget->currentIndex();

        load.cancel();  // 打开并发加载线程前关闭正在执行的线程
        m_loadweb->setValue(0);

        ui->label_more->clear();    // 清空详情介绍
        ui->label_info->clear();
        ui->label_appname->clear();
        ui->pushButton_download->setEnabled(false);
        ui->stackedWidget->setCurrentIndex(2);
        qDebug() << "https://demo-one-vert.vercel.app/" + type_name + "/" + pname;
        qDebug() << "链接地址：" << arg1;

        /*
        load.cancel();  // 打开并发加载线程前关闭正在执行的线程
        load = QtConcurrent::run([=]()
        {
            int loadresult = loadappinfo(arg1);
        });
        */
        appinfoLoadThread.requestInterruption();
        appinfoLoadThread.wait(100);
        appinfoLoadThread.setUrl(arg1);
        appinfoLoadThread.start();
    }
}

void Widget::on_webEngineView_loadStarted()
{
    m_loadweb->setValue(0);
    m_loadweb->show();
    m_loaderror->hide();
    ui->label_show->hide();
}

void Widget::on_webEngineView_loadProgress(int progress)
{
    m_loadweb->setValue(progress);
    if(progress >= 90)
    {
        m_loadweb->setValue(0);
    }
}

void Widget::on_webEngineView_loadFinished(bool arg1)
{
    if(arg1)
    {
        m_loadweb->setValue(0);
    }
    else
    {
        m_loadweb->setValue(0);
        m_loaderror->show();
    }
}

void Widget::on_pushButton_update_clicked()
{
    QDesktopServices::openUrl(QUrl("https://www.deepinos.org/"));
}

