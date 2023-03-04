#include "application.h"
#include "mainwindow-dtk.h"
#include "utils/utils.h"

#include <DPlatformWindowHandle>
#include <DLog>
#include <DGuiApplicationHelper>
#include <DAboutDialog>
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 4, 0))
#include <DFeatureDisplayDialog>
#endif

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

    // 获取版本特性信息
    m_featuresJsonObj = Utils::parseFeatureJsonFile();
}

void Application::handleAboutAction()
{
    if (aboutDialog() && aboutDialog()->parent() == m_mainWindow) {
        DApplication::handleAboutAction();
        return;
    }

    initAboutDialog();
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 4, 0))
    initFeatureDisplayDialog(); // 初始化版本特性对话框
#endif
    DApplication::handleAboutAction();
}

bool Application::notify(QObject *receiver, QEvent *event)
{
    if (m_mainWindow) {
        m_mainWindow->notify(receiver, event);
    }

    return DApplication::notify(receiver, event);
}

void Application::checkAppConfigLocation()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!dir.exists()) {
        qWarning() << "AppConfigLocation not existed, creating...";
        dir.mkpath(dir.absolutePath());
    }
}

void Application::setBuildDateTime(const QString &buildDateTime)
{
    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    if (config.value("build/version").toString() != QString(APP_VERSION)) {
        qDebug() << "Spark Store has been updated!";

        config.setValue("build/version", QString(APP_VERSION));
        config.setValue("build/time", buildDateTime);
        config.sync();
    }

    setApplicationVersion(DApplication::buildVersion(QString(APP_VERSION) + "-" + "Reason" + "-" + buildDateTime));
}

void Application::setMainWindow(MainWindow *window)
{
    m_mainWindow = window;
    if (aboutDialog() == nullptr || aboutDialog()->parent() != m_mainWindow)
    {
        initAboutDialog();
    }
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 4, 0))
    if (featureDisplayDialog() == nullptr || featureDisplayDialog()->parent() != m_mainWindow)
    {
        initFeatureDisplayDialog(); // 初始化版本特性对话框
    }
#endif
}

MainWindow *Application::mainWindow()
{
    return m_mainWindow;
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
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 4, 0))
    dialog->setVersion(applicationVersion());
#endif
    // 根据 shenmo 要求，不显示组织 Logo
    // dialog->setCompanyLogo(QPixmap(":/icon/Logo-Spark.png"));
    dialog->setCompanyLogo(QPixmap());
    dialog->setWebsiteName(QObject::tr("Spark Project"));
    dialog->setWebsiteLink(applicationHomePage());
    dialog->setDescription(applicationDescription());
    dialog->setLicense(translate("DAboutDialog", "%1 is released under %2").arg(productName()).arg(applicationLicense()));

    setAboutDialog(dialog);
    connect(aboutDialog(), &DAboutDialog::destroyed, this, [=]() {
        setAboutDialog(nullptr);
    });

    dialog->hide();
}

#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 4, 0))
/**
 * @brief Application::initFeatureDisplayDialog 初始化版本特性对话框
 */
void Application::initFeatureDisplayDialog()
{
    if (featureDisplayDialog())
    {
        featureDisplayDialog()->deleteLater();
        setFeatureDisplayDialog(nullptr);
    }

    // 自定义 DFeatureDisplayDialog
    DFeatureDisplayDialog *dialog = new DFeatureDisplayDialog(m_mainWindow);
    // 标题
    dialog->setTitle(m_featuresJsonObj.value("title").toString());
    // NOTE: json 文件中支持多语言；考虑到维护性，不放入翻译文件处理
    if (m_featuresJsonObj.contains(QString("title[%1]").arg(QLocale::system().name())))
    {
        dialog->setTitle(m_featuresJsonObj.value(QString("title[%1]").arg(QLocale::system().name())).toString());
    }

    // 特性项
    QList<DFeatureItem *> items;
    foreach (const QJsonValue &jsonValue, m_featuresJsonObj.value("items").toArray())
    {
        QJsonObject jsonObj = jsonValue.toObject();
        QString name = jsonObj.value("name").toString();
        if (jsonObj.contains(QString("name[%1]").arg(QLocale::system().name())))
        {
            name = jsonObj.value(QString("name[%1]").arg(QLocale::system().name())).toString();
        }
        QString description = jsonObj.value("description").toString();
        if (jsonObj.contains(QString("description[%1]").arg(QLocale::system().name())))
        {
            description = jsonObj.value(QString("description[%1]").arg(QLocale::system().name())).toString();
        }

        DFeatureItem *item = new DFeatureItem(QIcon::fromTheme("spark-store"), name, description, dialog);
        items.append(item);
    }
    dialog->addItems(items); // NOTE: 也支持 addItem 依次添加单个 item

    // “了解更多”链接按钮
    dialog->setLinkUrl(m_featuresJsonObj.value("linkUrl").toString());
    dialog->setLinkButtonVisible(m_featuresJsonObj.value("linkButtonVisible").toBool());

    setFeatureDisplayDialog(dialog);
    connect(featureDisplayDialog(), &DFeatureDisplayDialog::destroyed, this, [=]() {
        setFeatureDisplayDialog(nullptr);
    });
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 4, 0))
    connect(aboutDialog(), &DAboutDialog::featureActivated, this, [=]() {
        featureDisplayDialog()->show();
    });
#endif

    dialog->hide();
}
#endif
