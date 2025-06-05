#ifndef APTSSUPDATER_H
#define APTSSUPDATER_H

#include <QWidget>
#include <QStringList>
#include <QTemporaryFile>
#include <QLocale>
class aptssUpdater : public QWidget
{
    Q_OBJECT
public:
    explicit aptssUpdater(QWidget *parent = nullptr);

    QStringList getUpdateablePackages(); // 查询可更新包列表及更新内容
    qint64 getTotalDownloadSize();       // 获取更新总大小预估
    QStringList getPackageSizes();       // 获取每个包的大小
    QStringList getDesktopAppNames();  // 获取桌面应用名称列表
    QString m_tempFilePath;
signals:
private:
bool checkDesktopFiles(const QStringList &desktopFiles, QString &appName, const QString &lang, const QString &packageName);
};

#endif // APTSSUPDATER_H