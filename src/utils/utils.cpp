#include "utils.h"
#include "application.h"

#include <DSysInfo>

#include <QDBusInterface>
#include <QProcessEnvironment>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QUuid>
#include <QJsonDocument>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

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
 * @brief 判断是否使用 TreeLand 混合器
 * @return bool true: 使用 TreeLand 混合器 false: 非 TreeLand 混合器
 */
bool Utils::isTreeLand()
{
    bool isTreeLand = false;
    if (qgetenv("DDE_CURRENT_COMPOSITOR").toLower() == "treeland") {
        isTreeLand = true;
    }

    return isTreeLand;
}

bool Utils::isPhytium()
{
    bool isPhytium = false;
    QProcess process;
    process.start("lscpu");
    process.waitForFinished();


    QString output = process.readAllStandardOutput();
    if (output.contains(QLatin1String("Phytium")))
    {
        isPhytium = true;
    }

    return isPhytium;
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

    // check uuid
    if (!config.contains("info/uuid")){
        config.setValue("info/uuid", initUUID());
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
 * @brief Utils::initUUID 生成 UUID
*/
QString Utils::initUUID(){
    QUuid uuid = QUuid::createUuid();
    return uuid.toString();
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

    /**
     * NOTE: https://github.com/linuxdeepin/developer-center/issues/7217#issuecomment-1922653903
     * DDE Wayland has been deprecated, so using wayland plugin only
     */
    if (isWayland && useWayland /*&& !(Dtk::Core::DSysInfo::isDeepin() || isDDE)*/)
    {
        qputenv("QT_QPA_PLATFORM", "wayland");
    }
    // else if (isWayland && useWayland && (Dtk::Core::DSysInfo::isDeepin() && isDDE))
    // {
    //     qputenv("QT_QPA_PLATFORM", "dwayland");
    // }
    else
    {
        qputenv("QT_QPA_PLATFORM", "dxcb;xcb");
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

/**
 * @brief Utils::shouldDisableWebEngineSandbox 检查是否应关闭webengine沙箱
 * @return bool true: 配置文件中设置了关闭沙箱 false: 未设置或设置为false
 */
bool Utils::shouldDisableWebEngineSandbox()
{
    // WARNING: 请在 组织名称 和 应用程序名称 初始化完成后调用
    QSettings config(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat);
    
    // 检查配置文件中[webengine]部分的noSandbox配置项
    // 如果配置存在且值为true，则返回true；否则返回false
    qDebug()<<"sandbox is set to"<<config.value("webengine/noSandbox", false).toBool();
    return config.value("webengine/noSandbox", false).toBool();
}

// 日志相关静态变量
static QFile *logFile = nullptr;
static QString logFilePath;

// 自定义消息处理器，捕获所有Qt日志输出
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString level;
    
    switch (type) {
    case QtDebugMsg:
        level = "DEBUG";
        fprintf(stderr, "DEBUG: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        level = "INFO";
        fprintf(stderr, "INFO: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        level = "WARNING";
        fprintf(stderr, "WARNING: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        level = "ERROR";
        fprintf(stderr, "ERROR: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        level = "FATAL";
        fprintf(stderr, "FATAL: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
    
    // 写入到日志文件
    Utils::writeLog(level, msg);
}

// 初始化日志系统
void Utils::initLogger()
{
    // 确保日志目录存在
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir;
    if (!dir.exists(logDir)) {
        dir.mkpath(logDir);
    }
    
    // 设置日志文件路径
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    logFilePath = logDir + QString("/spark-store_%1.log").arg(timestamp);
    
    // 打开日志文件
    logFile = new QFile(logFilePath);
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open log file:" << logFilePath;
        delete logFile;
        logFile = nullptr;
        return;
    }
    
    // 安装自定义消息处理器，捕获所有Qt日志输出
    qInstallMessageHandler(customMessageHandler);
    
    // 写入日志头信息
    writeLog("INFO", "Logger initialized");
    writeLog("INFO", QString("Application started at %1").arg(
             QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
}

// 写入日志
void Utils::writeLog(const QString &level, const QString &message)
{
    if (!logFile || !logFile->isOpen()) {
        return;
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logEntry = QString("[%1] [%2] %3\n").arg(timestamp).arg(level).arg(message);
    
    QTextStream out(logFile);
    out << logEntry;
    logFile->flush();
    
    // 同时输出到控制台，便于调试
    if (level == "ERROR") {
        qCritical() << logEntry.trimmed();
    } else if (level == "WARNING") {
        qWarning() << logEntry.trimmed();
    } else {
        qDebug() << logEntry.trimmed();
    }
}

// 导出日志
bool Utils::exportLogs(const QString &targetPath)
{
    QString exportPath = targetPath;
    if (exportPath.isEmpty()) {
        exportPath = "/tmp/spark-store";
    }
    
    // 确保目标目录存在
    QDir dir;
    if (!dir.exists(exportPath)) {
        if (!dir.mkpath(exportPath)) {
            writeLog("ERROR", QString("Failed to create target directory: %1").arg(exportPath));
            return false;
        }
    }
    
    // 关闭当前日志文件，便于复制
    if (logFile && logFile->isOpen()) {
        logFile->close();
    }
    
    // 复制日志文件到目标位置
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString targetLogPath = exportPath + QString("/spark-store_full_log_%1.log").arg(timestamp);
    
    bool success = QFile::copy(logFilePath, targetLogPath);
    
    // 额外检查：即使QFile::copy返回false，也要检查目标文件是否实际存在且大小合理
    if (!success) {
        QFileInfo targetFileInfo(targetLogPath);
        if (targetFileInfo.exists() && targetFileInfo.size() > 0) {
            success = true;
        }
    }
    
    if (success) {
        writeLog("INFO", QString("All logs (INFO, DEBUG, WARNING, ERROR) exported to: %1").arg(targetLogPath));
        
        // 同时创建一个简单的导出报告
        QString reportPath = exportPath + QString("/export_report_%1.txt").arg(timestamp);
        QFile reportFile(reportPath);
        if (reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&reportFile);
            out << "Spark Store Log Export Report\n";
            out << "================================\n";
            out << "Export Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
            out << "Target Directory: " << exportPath << "\n";
            out << "Log File: " << targetLogPath << "\n";
            out << "Original Log: " << logFilePath << "\n";
            out << "Log Levels: INFO, DEBUG, WARNING, ERROR, FATAL\n";
            out << "Status: SUCCESS\n";
            reportFile.close();
        }
    } else {
        writeLog("ERROR", QString("Failed to export logs to: %1").arg(targetLogPath));
    }
    
    // 重新打开日志文件继续记录
    if (logFile && !logFile->isOpen()) {
        logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    }
    
    return success;
}

// 获取日志文件路径
QString Utils::getLogFilePath()
{
    return logFilePath;
}
