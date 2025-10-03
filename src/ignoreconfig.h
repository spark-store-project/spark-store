#ifndef IGNORECONFIG_H
#define IGNORECONFIG_H

#include <QObject>
#include <QSet>
#include <QString>
#include <QPair>

class IgnoreConfig : public QObject
{
    Q_OBJECT

public:
    explicit IgnoreConfig(QObject *parent = nullptr);

    // 添加忽略的应用（包名和版本号）
    void addIgnoredApp(const QString &packageName, const QString &version);
    
    // 移除忽略的应用
    void removeIgnoredApp(const QString &packageName);
    
    // 检查应用是否被忽略
    bool isAppIgnored(const QString &packageName, const QString &version) const;
    
    // 获取所有被忽略的应用
    QSet<QPair<QString, QString>> getIgnoredApps() const;
    
    // 输出所有被忽略的应用到 qDebug
    void printIgnoredApps() const;
    
    // 保存配置到文件
    bool saveConfig();
    
    // 从文件加载配置
    bool loadConfig();

private:
    QSet<QPair<QString, QString>> m_ignoredApps;
    QString m_configFilePath;
};

#endif // IGNORECONFIG_H