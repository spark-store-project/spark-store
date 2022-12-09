#include "mainwindow-dtk.h"

#include <DApplication>
#include <DLog>
#include <DPlatformWindowHandle>
#include <DApplicationSettings>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    //设置桌面环境环境变量
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    DApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // 开启 Hidpi 支持
#ifndef DSTORE_NO_DXCBs
    DApplication::loadDXcbPlugin(); // 加载 DXCB 插件
#endif

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

    a.setOrganizationName("spark-union");
    a.setProductName(QObject::tr("Spark Store"));
    a.setApplicationName("Spark Store"); // 不用翻译，影响 ~/.local/share/spark-union 下文件夹名称
    a.setApplicationDisplayName(QObject::tr("Spark Store")); // 设置窗口显示标题 (Wayland 下会显示 Qt 原生标题栏)
    a.setWindowIcon(QIcon::fromTheme("spark-store"));

    // 限制单实例运行
    if (!a.setSingleInstance("spark-store")) {
        return -1;
    }

    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType); //固定主题为浅色主题
    DApplicationSettings settings; // 定义 DApplicationSettings，自动保存主题设置

    MainWindow w;
    QString arg1 = argv[1];
    if (arg1.startsWith("spk://")) {
        w.openUrl(QUrl(argv[1]));
    }
    w.show();

    return a.exec();
}
