#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QMessageLogContext>
#include <QtGlobal>

class Utils
{
public:
    static void sendNotification(const QString &icon, const QString &title, const QString &body);
    static bool isDDE();
    static bool isWayland();
    static bool isTreeLand();
    static void initConfig();
    static bool isUOS();
    static bool isPhytium();
    static QString initUUID();
    static void setQPAPlatform();
    static void checkUOSDeveloperMode();
    static QJsonObject parseFeatureJsonFile();
    static bool shouldDisableWebEngineSandbox(); // 新增函数声明
    
    // 日志相关函数
    static void initLogger(); // 初始化日志系统
    static void writeLog(const QString &level, const QString &message); // 写入日志
    static bool exportLogs(const QString &targetPath = "/tmp/spark-store"); // 导出日志
    static QString getLogFilePath(); // 获取日志文件路径
};

#endif // UTILS_H