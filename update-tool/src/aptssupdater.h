#ifndef APTSSUPDATER_H
#define APTSSUPDATER_H

#include <QWidget>
#include <QStringList>
#include <QTemporaryFile>
#include <QLocale>
#include <QJsonObject>
#include <QJsonArray>
class aptssUpdater : public QWidget
{
    Q_OBJECT
public:
    explicit aptssUpdater(QWidget *parent = nullptr);

    QStringList getUpdateablePackages(); // 查询可更新包列表及更新内容
    QStringList getPackageSizes();       // 获取每个包的大小
    QStringList getDesktopAppNames();  // 获取桌面应用名称列表
    QStringList getPackageIcons();    // 获取包图标列表
    QJsonArray getUpdateInfoAsJson(); // 获取更新信息的 JSON 格式
    QString m_tempFilePath;
signals:
private:
    bool checkDesktopFiles(const QStringList &desktopFiles, QString &appName, const QString &lang, const QString &packageName);
    QStringList packageName;
};

#endif // APTSSUPDATER_H