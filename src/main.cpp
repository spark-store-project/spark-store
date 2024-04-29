#include "application.h"
#include "mainwindow-dtk.h"
#include "utils/utils.h"

#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <execinfo.h>

#include <DSysInfo>
#include <DApplicationSettings>
#include <DWidgetUtil>

#include <QDate>
#include <QProcessEnvironment>
#include <QSettings>
#include <QFile>
#include <QStandardPaths>
#include <QSurfaceFormat>

#include <backend/DataCollectorAndUploader.h>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static QString buildDateTime;


void gatherInfo(FILE *fp, std::ofstream& logFile, const char* description) {
    if (fp) {
        char buffer[512];
        logFile << description << ":\n";
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0;
            logFile << buffer << "\n";
        }
        pclose(fp);
    } else {
        logFile << "Failed to gather " << description << " info.\n";
    }
}


void crashHandler(int sig) {
    void *array[50];
    size_t size = backtrace(array, 50);
    if (size == 0) {
        perror("backtrace");
        exit(1);
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char filename[128];
    snprintf(filename, sizeof(filename), "/tmp/spark_store_crash_log_%04d%02d%02d_%02d%02d%02d.txt",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    std::ofstream logFile(filename, std::ios::out);

    if (!logFile.is_open()) {
        perror("ofstream");
        exit(1);
    }

    logFile << "Please send this log to the developer. QQ Group: 872690351\n";
    logFile << "Gitee: https://gitee.com/spark-store-project/spark-store/issues\n";
    logFile << "Gihub: https://github.com/spark-store-project/spark-store/issues\n";
    logFile << "Build Date and Time: " << buildDateTime.toStdString() << "\n";
    gatherInfo(popen("cat ~/.config/spark-union/spark-store/config.ini", "r"), logFile, "User Config File");

    // Collecting System Information
    gatherInfo(popen("LANG=en_US.UTF-8 uname -m", "r"), logFile, "CPU Architecture");
    gatherInfo(popen("LANG=en_US.UTF-8 lsb_release -a", "r"), logFile, "Distribution info");
    gatherInfo(popen("LANG=en_US.UTF-8 lscpu", "r"), logFile, "All CPU Info");
    gatherInfo(popen("LANG=en_US.UTF-8 free -h | grep Mem | awk '{print $2}'", "r"), logFile, "Memory Size");
  


    logFile << "Error: signal " << sig << ":\n";
    for (size_t i = 0; i < size; i++) {
        char **strings = backtrace_symbols(&array[i], 1);
        if (strings != NULL && strings[0] != NULL) {
            logFile << strings[0] << "\n";
        } else {
            logFile << "Failed to get symbol.\n";
        }
        free(strings);
    }

    logFile.close();

    char openCmd[256];
    snprintf(openCmd, sizeof(openCmd), "xdg-open %s", filename);
    if (system(openCmd) == -1) {
        perror("system");
    }

    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);

    exit(1);
}


int main(int argc, char *argv[])
{
    // 崩溃处理
    signal(SIGSEGV, crashHandler);  // 注册SIGSEGV处理函数


    // Get build time
    static const QDate buildDate = QLocale(QLocale::English).toDate(QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
    static const QTime buildTime = QTime::fromString(__TIME__, "hh:mm:ss");
    buildDateTime = buildDate.toString("yyyy.MM.dd") + "-" + buildTime.toString("hh:mm:ss");


    // NOTE: 提前设置组织名称和应用名称，避免配置文件位置错误
    DApplication::setOrganizationName("spark-union");
    DApplication::setApplicationName("spark-store");
    Application::checkAppConfigLocation(); // 检查 ~/.config/spark-union/spark-store 文件夹是否存在

    // 初始化 config.ini 配置文件
    Utils::initConfig();

    // 回传版本信息，不涉及个人隐私
    DataCollectorAndUploader uploader;
    QObject::connect(&uploader, &DataCollectorAndUploader::uploadSuccessful, [](){
        qDebug() << "Data uploaded successfully";
    });
    QObject::connect(&uploader, &DataCollectorAndUploader::uploadFailed, [](QString error){
        qDebug() << "Upload failed with error: " << error;
    });

    uploader.collectAndUploadData();

    // Set display backend
    Utils::setQPAPlatform();

    // 龙芯机器配置,使得 DApplication 能正确加载 QtWebEngine
    qputenv("DTK_FORCE_RASTER_WIDGETS", "FALSE");

    // 浏览器开启 GPU 支持
    // qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-features=UseModernMediaControls");
    // qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-web-security");
#ifdef __sw_64__
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox");
#endif

#ifdef __loongarch__
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox");
#endif
    /**
     * NOTE: https://zhuanlan.zhihu.com/p/550285855
     * 避免 wayland 环境下从 QtWebEngine 后退回到 QWidget 时黑屏闪烁
     */
    if (Utils::isWayland()) {
        qputenv("QMLSCENE_DEVICE", "softwarecontext");
        DApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    }



#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    // 开启 Hidpi 支持
    qDebug() << "Enable HiDPI Support.";
    DApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    DApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
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
