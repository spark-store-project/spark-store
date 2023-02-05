#include "application.h"
#include "mainwindow-dtk.h"

#include <DPlatformWindowHandle>
#include <DLog>
#include <DGuiApplicationHelper>
#include <DAboutDialog>

#include <QSettings>
#include <QStandardPaths>

DCORE_USE_NAMESPACE

Application::Application(int &argc, char **argv)
    : DApplication(argc, argv)
{
    // Wayland 环境下使用，防止子控件 Native 化
    if (!DPlatformWindowHandle::pluginVersion().isEmpty()) {
        setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    }
    setAttribute(Qt::AA_UseHighDpiPixmaps);

    setQuitOnLastWindowClosed(false); // 启用托盘图标时，关闭窗口程序仍然运行

    loadTranslator(); // 载入翻译

    setOrganizationName("spark-union");
    setApplicationName("spark-store"); // 影响 ~/.config/spark-union ~/.local/share/spark-union 下文件夹名称
    setApplicationDisplayName(QObject::tr("Spark Store")); // 设置窗口显示标题 (Wayland 下会显示 Qt 原生标题栏)
    setProductName(QObject::tr("Spark Store"));
    setProductIcon(QIcon::fromTheme("spark-store"));
    setApplicationHomePage("https://gitee.com/deepin-community-store");
    setApplicationDescription(
        QObject::tr(
            "<span style=' font-size:10pt;font-weight:60;'>An appstore powered by community</span><br/>"
            "<a href='https://www.spark-app.store/'>https://www.spark-app.store</a><br/>"
            "<span style=' font-size:12pt;'>Spark developers</span>"));
    setApplicationLicense(" <a href='https://gitee.com/deepin-community-store/spark-store/blob/Flamescion/LICENSE'>GPLv3</a> ");

    // 检查 ~/.config/spark-union/spark-store 文件夹是否存在
    checkAppConfigLocation();

    // 初始化日志模块 (默认日志位置 ~/.cache/spark-union/spark-store)
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
}

void Application::handleAboutAction()
{
    if (aboutDialog() && aboutDialog()->parent() == m_mainWindow) {
        DApplication::handleAboutAction();
        return;
    }

    initAboutDialog();
    DApplication::handleAboutAction();
}

void Application::checkAppConfigLocation()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!dir.exists()) {
        qWarning() << "AppConfigLocation not existed, creating...";
        dir.mkpath(dir.absolutePath());
    }
}

void Application::setVersionAndBuildDateTime(const QString &version, const QString &buildDateTime)
{
    m_version = version;
    m_buildDateTime = buildDateTime;

    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    if (config.value("build/version").toString() != m_version) {
        qDebug() << "Spark Store has been updated!";

        config.setValue("build/version", m_version);
        config.setValue("build/time", m_buildDateTime);
        config.sync();
    }

    setApplicationVersion(DApplication::buildVersion(config.value("build/version").toString() + "-" + "Flamescion" + "-" + config.value("build/time").toString()));
}

void Application::setMainWindow(MainWindow *window)
{
    m_mainWindow = window;
    if (aboutDialog() == nullptr || aboutDialog()->parent() != m_mainWindow)
    {
        initAboutDialog();
    }
}

void Application::initAboutDialog()
{
    if (m_mainWindow == nullptr)
    {
        return;
    }

    if (aboutDialog())
    {
        aboutDialog()->deleteLater();
        setAboutDialog(nullptr);
    }

    // 自定义 DAboutDialog
    DAboutDialog *dialog = new DAboutDialog(m_mainWindow);
    dialog->setProductName(productName());
    dialog->setProductIcon(productIcon());
    dialog->setVersion(translate("DAboutDialog", "Version: %1").arg(applicationVersion()));
    // 根据 shenmo 要求，不显示组织 Logo
    // dialog->setCompanyLogo(QPixmap(":/icon/Logo-Spark.png"));
    dialog->setCompanyLogo(QPixmap());
    dialog->setWebsiteName(QObject::tr("Spark Project"));
    dialog->setWebsiteLink(applicationHomePage());
    dialog->setDescription(applicationDescription());
    dialog->setLicense(translate("DAboutDialog", "%1 is released under %2").arg(productName()).arg(applicationLicense()));

    setAboutDialog(dialog);
    connect(aboutDialog(), &DAboutDialog::destroyed, this, [=] {
        setAboutDialog(nullptr);
    });

    dialog->hide();
}
