#include "aptssupdater.h"
#include <QProcess>
#include <QTextStream>
#include <QRegularExpression>
#include <QFile>

aptssUpdater::aptssUpdater(QWidget *parent)
    : QWidget(parent)
{

}

QStringList aptssUpdater::getUpdateablePackages()
{
    QStringList packageDetails;
    QProcess process;
    QString command = R"(env LANGUAGE=en_US /usr/bin/apt -c /opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf list --upgradable -o Dir::Etc::sourcelist="/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list" -o Dir::Etc::sourceparts="/dev/null" -o APT::Get::List-Cleanup="0" | awk 'NR>1')";
    
    process.start("bash", QStringList() << "-c" << command);
    if (!process.waitForFinished()) {
        qWarning() << "Process failed to finish.";
        return packageDetails;
    }

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    // 创建临时文件
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    if (tempFile.open()) {
        QTextStream stream(&tempFile);
        
        for (const QString &line : lines) {
            QRegularExpression regex(R"(([\w\-\+\.]+)/\S+\s+([^\s]+)\s+\S+\s+\[upgradable from: ([^\]]+)\])");
            QRegularExpressionMatch match = regex.match(line);
            if (match.hasMatch()) {
                QString name = match.captured(1);
                QString newVersion = match.captured(2);
                QString oldVersion = match.captured(3);
                
                // 写入内存列表
                packageDetails << QString("%1: %2 → %3").arg(name, oldVersion, newVersion);
                
                // 写入临时文件（原始数据）
                stream << name << "|" << oldVersion << "|" << newVersion << "\n";
            }
        }
        tempFile.close();
        m_tempFilePath = tempFile.fileName();
        qDebug()<< "临时文件路径：" << m_tempFilePath;
        
    } else {
        qWarning() << "无法创建临时文件";
    }

    return packageDetails;
}


QStringList aptssUpdater::getPackageSizes()
{

}

QStringList aptssUpdater::getDesktopAppNames()
{
    QStringList appNames;
    QProcess dpkgProcess;
    
    // 获取当前系统语言环境
    QString lang = QLocale().name().replace("_", "-");

    // 遍历所有可更新包（复用已有的临时文件数据）
    QStringList packages = getUpdateablePackages();
    
    foreach (const QString &package, packages) {
        QString packageName = package.split(":")[0];
        
        // 获取包文件列表
        dpkgProcess.start("dpkg", QStringList() << "-L" << packageName);
        dpkgProcess.waitForFinished();
        QStringList files = QString(dpkgProcess.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);

        // 检查常规应用目录
        checkDesktopFiles(files.filter("/usr/share/applications/"), 
                       appNames, 
                       lang, 
                       packageName);  // 新增包名参数
        
        // 检查特殊目录（/opt/apps）
        checkDesktopFiles(files.filter(QRegularExpression(QString("/opt/apps/%1/entries/applications").arg(packageName))),
                        appNames,
                        lang,
                        packageName);  // 新增包名参数
        
    }
    
    return appNames;
}

bool aptssUpdater::checkDesktopFiles(const QStringList &desktopFiles, QStringList &appNames, const QString &lang, const QString &packageName)
{
    bool hasFoundName = false;
    QRegularExpression noDisplayRe("^NoDisplay=(true|True)");
    QRegularExpression nameRe("^Name\\[?" + lang + "?\\]?=(.*)");
    QRegularExpression nameOrigRe("^Name=(.*)");

    foreach (const QString &filePath, desktopFiles) {
        if (!filePath.endsWith(".desktop")) continue;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        bool skip = false;
        QString name;
        
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            
            // 检查NoDisplay属性
            if (line.startsWith("NoDisplay=")) {
                if (noDisplayRe.match(line).hasMatch()) {
                    skip = true;
                    break;
                }
            }
            
            // 优先匹配本地化名称
            if (name.isEmpty()) {
                QRegularExpressionMatch match = nameRe.match(line);
                if (match.hasMatch()) {
                    name = match.captured(1);
                    continue;
                }
                
                // 匹配原始名称
                match = nameOrigRe.match(line);
                if (match.hasMatch()) {
                    name = match.captured(1);
                }
            }
        }
        
        if (!skip && !name.isEmpty() && !appNames.contains(name)) {
            appNames << name;
            hasFoundName = true;  // 标记已找到有效名称
        }
    }

    // 如果没有找到任何名称，则使用包名
    if (!hasFoundName && !appNames.contains(packageName)) {
        appNames << packageName;
    }

    return hasFoundName;
}