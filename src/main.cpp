#include "application.h"
#include "mainwindow-dtk.h"

#include <DSysInfo>
#include <DApplicationSettings>
#include <DWidgetUtil>

#include <QStandardPaths>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#define UOSCheckFile "/var/lib/deepin/developer-mode/enabled"

int main(int argc, char *argv[])
{
    // Get build time
    static const QString version = "4.1.2";
    static const QDate buildDate = QLocale(QLocale::English).toDate(QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
    static const QTime buildTime = QTime::fromString(__TIME__, "hh:mm:ss");
    static const QString buildDateTime = buildDate.toString("yyyy.MM.dd") + "-" + buildTime.toString("hh:mm:ss");

    // 设置桌面环境环境变量
    bool isDeepinOS = true;
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin"))
    {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
        isDeepinOS = false;
    }

    bool isWayland = false;
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive))
    {
        isWayland = true;
    }

    // NOTE: 提前设置组织名称和应用名称，避免配置文件位置错误
    DApplication::setOrganizationName("spark-union");
    DApplication::setApplicationName("spark-store");
    Application::checkAppConfigLocation(); // 检查 ~/.config/spark-union/spark-store 文件夹是否存在

    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    config.setValue("build/isWayland", isWayland);
    config.setValue("build/isDeepinOS", isDeepinOS);
    // Check config file, if there is no wayland config, then set it to default, which means use wayland if possible.
    if (!config.contains("build/useWayland"))
    {
        config.setValue("build/useWayland", true);
    }
    config.sync(); // 写入更改至 config.ini，并同步最新内容

    bool useWayland = config.value("build/useWayland").toBool();
    qDebug() << "System Wayland enabled:" << isWayland << ". Spark Wayland enabled:" << useWayland;

    // Set display backend
    if (isWayland && useWayland && !(Dtk::Core::DSysInfo::isDDE() || isDeepinOS))
    {
        qputenv("QT_QPA_PLATFORM", "wayland");
    }
    else if (isWayland && useWayland && (Dtk::Core::DSysInfo::isDDE() && isDeepinOS))
    {
        qputenv("QT_QPA_PLATFORM", "dwayland");
    }
    else
    {
        qputenv("QT_QPA_PLATFORM", "dxcb");
    }

    // Check UOS developer mode.
    QFile UOSDevelopFile(UOSCheckFile);
    if (UOSDevelopFile.exists() && isDeepinOS)
    {
        config.setValue("UOS/isUOS", true);
        QTextStream UOStextStream(&UOSDevelopFile);
        QString lineData = UOStextStream.readLine();
        bool devmode = lineData.toInt();
        qDebug() << "UOS Developer Mode Status:" << devmode;
        config.setValue("UOS/EnableDeveloperMode", devmode);
    }
    else
    {
        if (config.contains("UOS/isUOS"))
        {
            config.remove("UOS/isUOS");
            config.remove("UOS/EnableDeveloperMode");
        }
    }
    config.sync(); // 写入更改至 config.ini，并同步最新内容

    // 龙芯机器配置,使得 DApplication 能正确加载 QTWEBENGINE
    qputenv("DTK_FORCE_RASTER_WIDGETS", "FALSE");
//    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-features=UseModernMediaControls");
//    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-web-security");
    // 浏览器开启 GPU 支持
#ifdef __sw_64__
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--ignore-gpu-blocklist --enable-gpu-rasterization --enable-native-gpu-memory-buffers --enable-accelerated-video-decode --no-sandbox");
#else
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--ignore-gpu-blocklist --enable-gpu-rasterization --enable-native-gpu-memory-buffers --enable-accelerated-video-decode");
#endif

    DApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // 开启 Hidpi 支持

    // 强制使用 DTK 平台插件
    QVector<char *> fakeArgs(argc + 2);
    fakeArgs[0] = argv[0];
    fakeArgs[1] = const_cast<char *>("-platformtheme");
    fakeArgs[2] = const_cast<char *>("deepin");
    for (int i = 1; i < argc; i++)
    {
        fakeArgs[i + 2] = argv[i];
    }
    int fakeArgc = argc + 2; // QCoreApplication 的 argc 要用引用，避免 c++ 编译器优化
    Application a(fakeArgc, fakeArgs.data());
    // 设置版本和构建时间
    a.setVersionAndBuildDateTime(version, buildDateTime);

    // 限制单实例运行
    if (!a.setSingleInstance("spark-store"))
    {
        qWarning() << "Another instance has already started, activating...";
        return -1;
    }

    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType); // 固定主题为浅色主题
    DApplicationSettings settings; // 定义 DApplicationSettings，自动保存主题设置

    MainWindow w;
    // 让打开时界面显示在正中
    Dtk::Widget::moveToCenter(&w);

    if (argc > 1)
    {
        QString arg1 = argv[1];
        if (arg1.trimmed().startsWith("spk://"))
        {
            w.openUrl(QUrl(argv[1]));
        }
    }
    w.show();

    return a.exec();
}
