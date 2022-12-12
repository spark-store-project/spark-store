#include "mainwindow-dtk.h"

#include <DApplication>
#include <DLog>
#include <DPlatformWindowHandle>
#include <DApplicationSettings>
#include <DAboutDialog>
#include <QLabel>
#include <DWidgetUtil>
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    // Get build time
    static const QString version = "Version 4.0.0~test1";
    static const QDate buildDate = QLocale( QLocale::English ).toDate( QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
    static const QTime buildTime = QTime::fromString(__TIME__, "hh:mm:ss");

    //设置桌面环境环境变量
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    DApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // 开启 Hidpi 支持
#ifndef DSTORE_NO_DXCBs
    DApplication::loadDXcbPlugin(); // 加载 DXCB 插件
#endif

    // 浏览器开启 GPU 支持
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--ignore-gpu-blocklist --enable-gpu-rasterization --enable-native-gpu-memory-buffers --enable-accelerated-video-decode");

    //初始化日志模块 (默认日志位置 ~/.cache/deepin/spark-store)
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    DApplication a(argc, argv);

    //Wayland 环境下使用，防止子控件 Native 化
    if (!DPlatformWindowHandle::pluginVersion().isEmpty()) {
        a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    }
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.loadDXcbPlugin();

    a.loadTranslator(); // 载入翻译

    QSettings readConfig(QDir::homePath() + "/.config/spark-store/config.ini", QSettings::IniFormat);

    if (readConfig.value("build/version").toString() != version){
        qDebug() << "Spark Store has been updated!";
        QSettings *setConfig = new QSettings(QDir::homePath() + "/.config/spark-store/config.ini", QSettings::IniFormat);
        setConfig->setValue("build/version", version);
        setConfig->setValue("build/time", buildDate.toString("yyyy.MM.dd")+"-"+buildTime.toString());
        setConfig->deleteLater();
    }

    //Customized DAboutDialog

     DAboutDialog dialog;
      a.setAboutDialog(&dialog);
      dialog.setLicense(QObject::tr("We publish this program under GPL V3"));
      dialog.setVersion(DApplication::buildVersion(readConfig.value("build/version").toString()+"-"+readConfig.value("build/time").toString()));
      dialog.setProductIcon(QIcon::fromTheme("spark-store"));  // 设置Logo
      dialog.setProductName(QLabel::tr("Spark Store"));
      dialog.setDescription(
                  QObject::tr(
                      "<span style=' font-size:10pt;font-weight:60;'>An appstore powered by  community</span><br/>"
                      "<a href='https://www.spark-app.store/'>https://www.spark-app.store</a><br/>"
                      "<span style=' font-size:12pt;'>Spark developers</span>"
                      )
                  );

      dialog.setProductName(QLabel::tr("Spark Store"));
      dialog.setCompanyLogo(QPixmap(":/spark-store.png"));
      dialog.setWebsiteName(QObject::tr("The Spark Project"));
      dialog.setWebsiteLink("https://gitee.com/deepin-community-store");

    a.setOrganizationName("spark-union");
    a.setProductName(QObject::tr("Spark Store"));
    a.setApplicationName("Spark Store"); // 不用翻译，影响 ~/.local/share/spark-union 下文件夹名称
    a.setApplicationDisplayName(QObject::tr("Spark Store")); // 设置窗口显示标题 (Wayland 下会显示 Qt 原生标题栏)
    a.setWindowIcon(QIcon::fromTheme("spark-store"));
    a.setApplicationDescription(
                QObject::tr(
                    "<span style='font-size:10pt;font-weight:60;'>An appstore powered by deepin community</span><br/>"
                    "<a href='https://www.spark-app.store/'>https://www.spark-app.store</a><br/>"
                    "<span style='font-size:12pt;'>Spark developers</span><br/><br/>"
                    "Published under GPL V3"
                    )
                );
    // 限制单实例运行
    if (!a.setSingleInstance("spark-store")) {
        return -1;
    }

    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType); //固定主题为浅色主题
    DApplicationSettings settings; // 定义 DApplicationSettings，自动保存主题设置

    MainWindow w;
    // 让打开时界面显示在正中
    Dtk::Widget::moveToCenter(&w);

    QString arg1 = argv[1];
    if (arg1.startsWith("spk://")) {
        w.openUrl(QUrl(argv[1]));
    }
    w.show();

    return a.exec();
}
