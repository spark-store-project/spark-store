#include "application.h"
#include "mainwindow-dtk.h"
#include "utils/utils.h"

#include <DSysInfo>
#include <DApplicationSettings>
#include <DWidgetUtil>

#include <QDate>
#include <QProcessEnvironment>
#include <QSettings>
#include <QFile>
#include <QStandardPaths>
#include <QSurfaceFormat>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    // Get build time
    static const QDate buildDate = QLocale(QLocale::English).toDate(QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
    static const QTime buildTime = QTime::fromString(__TIME__, "hh:mm:ss");
    static const QString buildDateTime = buildDate.toString("yyyy.MM.dd") + "-" + buildTime.toString("hh:mm:ss");

    // NOTE: 提前设置组织名称和应用名称，避免配置文件位置错误
    DApplication::setOrganizationName("spark-union");
    DApplication::setApplicationName("spark-store");
    Application::checkAppConfigLocation(); // 检查 ~/.config/spark-union/spark-store 文件夹是否存在

    // 初始化 config.ini 配置文件
    Utils::initConfig();

    // Set display backend
    Utils::setQPAPlatform();

    // 龙芯机器配置,使得 DApplication 能正确加载 QTWEBENGINE
    qputenv("DTK_FORCE_RASTER_WIDGETS", "FALSE");
    // qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-features=UseModernMediaControls");
    // qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-web-security");


    // 浏览器开启 GPU 支持
#ifdef __sw_64__
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox");
#elif __aarch64__
    if (!Utils::isWayland()) {
        QString env = QString::fromUtf8(qgetenv("QTWEBENGINE_CHROMIUM_FLAGS"));
        env = env.trimmed();
        /**
         * NOTE: 参考帮助手册代码，对于部分ARM CPU 设备，
         * --disable-gpu 保证 X11 环境下网页正常显示
         * --single-process 避免 X11 环境下 QtWebEngine 崩溃（可选）
         */
        env += " --disable-gpu";
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", env.trimmed().toUtf8());

        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        QSurfaceFormat::setDefaultFormat(format);

        /**
         * NOTE: https://zhuanlan.zhihu.com/p/550285855
         * 避免 X11 环境下从 QtWebEngine 后退回到 QWidget 时黑屏闪烁
         */
        qputenv("QMLSCENE_DEVICE", "softwarecontext");
        DApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    }
#endif



    /**
     * FIXME: 对于麒麟 CPU 设备，调用 QtWebEngine 会导致客户端崩溃；
     * 暂时不对 CPU 进行判断，对 wayland 环境下统一处理
     */
    if (Utils::isWayland()) {
        QString env = QString::fromUtf8(qgetenv("QTWEBENGINE_CHROMIUM_FLAGS"));
        env = env.trimmed();
        /**
         * NOTE: 参考帮助手册代码，对于麒麟 CPU 设备，
         * --disable-gpu 保证 wayland 环境下网页正常显示
         * --single-process 避免 wayland 环境下 QtWebEngine 崩溃（可选）
         */
        env += " --disable-gpu";
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", env.trimmed().toUtf8());

        /**
         * NOTE: 参考帮助手册代码，对于麒麟 CPU 设备，
         * 避免 wayland 环境下 QtWebEngine 崩溃
         */
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        QSurfaceFormat::setDefaultFormat(format);

        /**
         * NOTE: https://zhuanlan.zhihu.com/p/550285855
         * 避免 wayland 环境下从 QtWebEngine 后退回到 QWidget 时黑屏闪烁
         */
        qputenv("QMLSCENE_DEVICE", "softwarecontext");
        DApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    {
        // 开启 Hidpi 支持
        qDebug() << "Enable HiDPI Support.";
        DApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        DApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    }
#endif

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
    a.setBuildDateTime(buildDateTime);

    // 限制单实例运行
    if (!a.setSingleInstance("spark-store"))
    {
        qWarning() << "Another instance has already started, activating...";
        return -1;
    }

    DApplicationSettings settings; // 定义 DApplicationSettings，自动保存主题设置

    MainWindow w;
    a.setMainWindow(&w); // 设置应用程序主窗口，用于初始化关于对话框
    // 让打开时界面显示在正中
    Dtk::Widget::moveToCenter(&w);

    if (argc > 1)
    {
        QString arg1 = argv[1];
        if (arg1.trimmed().startsWith("spk://"))
        {
            w.openUrl(arg1);
        }
    }
    w.show();

    return a.exec();
}
