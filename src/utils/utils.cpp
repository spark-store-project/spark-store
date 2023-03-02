#include "utils.h"
#include "application.h"

#include <DSysInfo>

#include <QDBusInterface>
#include <QProcessEnvironment>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>

#define UOSDeveloperModeFile "/var/lib/deepin/developer-mode/enabled"

/**
 * @author chatGPT
 * @brief Utils::sendNotification 发送系统通知
 * @param icon 图标名称
 * @param title 通知标题
 * @param body 通知内容
 */
void Utils::sendNotification(const QString &icon, const QString &title, const QString &body)
{
    QDBusInterface interface("org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications");

    QVariantList args;
    args << Application::applicationName() // the name of the application
         << static_cast<quint32>(0)        // replaces the previous notification with the same ID
         << icon                           // the application icon of the notification
         << title                          // the title of the notification
         << body                           // the body of the notification
         << QStringList()                  // no actions
         << QVariantMap()                  // no hints
         << -1;                            // no timeout

    interface.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
}

/**
 * @brief Utils::isDDE 判断是否使用 DDE 桌面环境
 * @return
 */
bool Utils::isDDE()
{
    bool isDDE = true;
    if (!QString::fromUtf8(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin"))
    {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
        isDDE = false;
    }

    return isDDE;
}

/**
 * @brief Utils::isWayland 判断是否使用 wayland 显示协议
 * @return bool true: 使用 wayland 显示协议 false: 使用 x11 显示协议
 */
bool Utils::isWayland()
{
    bool isWayland = false;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = env.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = env.value(QStringLiteral("WAYLAND_DISPLAY"));
    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive))
    {
        isWayland = true;
    }

    return isWayland;
}

/**
 * @brief Utils::initConfig 初始化 config.ini 配置文件，去除废弃字段
 */
void Utils::initConfig()
{
    // WARNING: 请在 组织名称 和 应用程序名称 初始化完成后调用
    bool useWayland = true;

    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    if (config.contains("build/useWayland"))
    {
        useWayland = config.value("build/useWayland").toBool();
    }
    config.remove("build/isDeepinOS");
    config.remove("build/isWayland");
    config.remove("build/useWayland");

    config.setValue("runtime/isDDE", isDDE());
    config.setValue("runtime/isWayland", isWayland());
    // Check config file, if there is no wayland config, then set it to default, which means use wayland if possible.
    if (!config.contains("runtime/useWayland"))
    {
        config.setValue("runtime/useWayland", useWayland);
    }
    config.sync(); // 写入更改至 config.ini，并同步最新内容

    checkUOSDeveloperMode(); // Check UOS developer mode
}

/**
 * @brief Utils::isUOS 判断是否为 UOS 操作系统
 * @return bool true: UOS 操作系统 false: 非 UOS 操作系统
 */
bool Utils::isUOS()
{
    // WARNING: 请在 组织名称 和 应用程序名称 初始化完成后调用
    bool isUOS = false;

    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    if (Dtk::Core::DSysInfo::productType() == Dtk::Core::DSysInfo::Uos)
    {
        isUOS = true;
        config.setValue("UOS/isUOS", true);
    }
    else
    {
        isUOS = false;
        config.remove("UOS");
    }
    config.sync(); // 写入更改至 config.ini，并同步最新内容

    return isUOS;
}

/**
 * @brief Utils::setQPAPlatform Set display backend
 */
void Utils::setQPAPlatform()
{
    // WARNING: 请在 initConfig() 执行后调用
    bool isDDE = Utils::isDDE(); // WARNING: 只能执行一次，后续执行时环境变量已经被覆盖，导致判断错误
    bool isWayland = Utils::isWayland();

    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    bool useWayland = config.value("runtime/useWayland").toBool();

    qDebug() << "System Wayland enabled:" << isWayland << "Spark Wayland enabled:" << useWayland;

    if (isWayland && useWayland && !(Dtk::Core::DSysInfo::isDeepin() || isDDE))
    {
        qputenv("QT_QPA_PLATFORM", "wayland");
    }
    else if (isWayland && useWayland && (Dtk::Core::DSysInfo::isDeepin() && isDDE))
    {
        qputenv("QT_QPA_PLATFORM", "dwayland");
    }
    else
    {
        qputenv("QT_QPA_PLATFORM", "dxcb");
    }
}

/**
 * @brief Utils::checkUOSDeveloperMode Check UOS developer mode
 */
void Utils::checkUOSDeveloperMode()
{
    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);

    QFile file(UOSDeveloperModeFile);
    if (Utils::isUOS() && file.exists() && file.open(QFile::ReadOnly | QFile::Text))
    {
        QString lineData = QString::fromUtf8(file.readLine());
        bool devmode = lineData.trimmed().toInt();
        qDebug() << "UOS Developer Mode Status:" << devmode;
        config.setValue("UOS/EnableDeveloperMode", devmode);
    }
    else
    {
        /**
         * NOTE: Utils::isUOS() 中，判断为非 UOS 时已经从 config 中删除 UOS 组，
         * 此处若包含该字段则一定是 UOS，直接写入开发者模式开关状态即可
         */
        if (config.contains("UOS/EnableDeveloperMode"))
        {
            config.setValue("UOS/EnableDeveloperMode", false);
        }
    }
    file.close();
    config.sync(); // 写入更改至 config.ini，并同步最新内容
}

/**
 * @brief Utils::parseFeatureJsonFile 解析版本特性 json 文件
 * @return 返回 QJsonObject
 */
QJsonObject Utils::parseFeatureJsonFile()
{
    QFile file(":/json/features.json");
    if (!file.open(QFile::ReadOnly))
    {
        qWarning() << Q_FUNC_INFO << "features.json open failed";
        return QJsonObject();
    }

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || jsonDoc.isNull())
    {
        qWarning() << Q_FUNC_INFO << "features.json validate failed:" << error.errorString();
        return QJsonObject();
    }

    if (jsonDoc.isEmpty() || !jsonDoc.isObject())
    {
        qWarning() << Q_FUNC_INFO << "features jsonDoc parse failed:" << jsonDoc;
        return QJsonObject();
    }

    return jsonDoc.object();
}
