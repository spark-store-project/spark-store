#include "aptssupdater.h"
#include <QProcess>
#include <QTextStream>
#include <QRegularExpression>
#include <QFile>
#include <qdebug.h>

aptssUpdater::aptssUpdater(QWidget *parent)
    : QWidget(parent)
{
    packageName = getUpdateablePackages();
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
                
                // 检查版本是否相同，相同则跳过
                if (newVersion == oldVersion) {
                    qDebug() << "跳过版本相同的包:" << name << "(" << oldVersion << "→" << newVersion << ")";
                    continue;
                }
                
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
    QStringList packageSizes;
    QProcess process;

    // 使用绝对路径 + 自动输入N跳过交互
    QString command = R"(echo N | /opt/durapps/spark-store/bin/aptss upgrade --assume-no)";

    process.start("bash", QStringList() << "-c" << command);
    if (!process.waitForFinished()) {
        qWarning() << "获取包大小失败：进程未完成";
        return packageSizes;
    }

    QString output = process.readAllStandardOutput() + process.readAllStandardError();

    // 清理颜色控制字符（ANSI 转义序列）
    output.remove(QRegularExpression(R"(\x1B\[[0-9;]*[a-zA-Z])"));  // \x1B is ESC

    QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    // 从 packageName 获取包名
    QStringList updateablePackages;
    for (const QString &pkgInfo : packageName) {
        QString pkgName = pkgInfo.section(":", 0, 0).trimmed();
        updateablePackages << pkgName;
    }

    // 匹配 [apt-fast xx:xx:xx] 行，提取内容
    QRegularExpression sizeLineRegex(R"(\[apt-fast [^\]]+\](\S+)\s+\S+\s+(\d+(\.\d+)?[KMG]?iB))");

    for (const QString &line : lines) {
        QRegularExpressionMatch match = sizeLineRegex.match(line.trimmed());
        if (match.hasMatch()) {
            QString name = match.captured(1);
            QString size = match.captured(2);

            if (updateablePackages.contains(name)) {
                packageSizes << QString("%1: %2").arg(name, size);
            }
        }
    }

    qDebug() << "包大小列表：" << packageSizes;
    return packageSizes;
}





QStringList aptssUpdater::getDesktopAppNames()
{
    QStringList appNames;
    QProcess dpkgProcess;
    
    // 获取当前系统语言环境
    QString lang = QLocale().name().replace("_", "-");

    // 遍历所有可更新包（复用已有的临时文件数据）
    QStringList packages = packageName;
    
    foreach (const QString &package, packages) {
        QString packageName = package.split(":")[0];
        QString finalName = packageName; // 默认使用包名
        
        // 获取包文件列表
        dpkgProcess.start("dpkg", QStringList() << "-L" << packageName);
        dpkgProcess.waitForFinished();
        QStringList files = QString(dpkgProcess.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);

        // 先检查常规应用目录
        QStringList regularDesktopFiles = files.filter("/usr/share/applications/");
        QString regularAppName;
        if (!regularDesktopFiles.isEmpty()) {
            checkDesktopFiles(regularDesktopFiles, regularAppName, lang, packageName);
        }
        
        // 如果常规目录没有找到，再检查特殊目录
        if (regularAppName.isEmpty()) {
            QStringList specialDesktopFiles = files.filter(QRegularExpression(QString("/opt/apps/%1/entries/applications").arg(packageName)));
            QString specialAppName;
            if (!specialDesktopFiles.isEmpty()) {
                checkDesktopFiles(specialDesktopFiles, specialAppName, lang, packageName);
                if (!specialAppName.isEmpty()) {
                    finalName = specialAppName;
                }
            }
        } else {
            finalName = regularAppName;
        }
        
        // 输出格式为[软件名|包名]
        appNames << QString("[%1|%2]").arg(finalName, packageName);
    }
    qDebug()<< "应用名称列表：" << appNames;
    return appNames;
}


bool aptssUpdater::checkDesktopFiles(const QStringList &desktopFiles, QString &appName, const QString &lang, const QString &packageName)
{
    QString lastValidName; 
    QRegularExpression noDisplayRe("^NoDisplay=(true|True)");
    QRegularExpression nameRe("^Name\\[?" + lang + "?\\]?=(.*)");
    QRegularExpression nameOrigRe("^Name=(.*)");

    foreach (const QString &filePath, desktopFiles) {
        if (!filePath.endsWith(".desktop")) continue;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        bool skip = false;
        QString currentName;
        
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
            if (currentName.isEmpty()) {
                QRegularExpressionMatch match = nameRe.match(line);
                if (match.hasMatch()) {
                    currentName = match.captured(1);
                    continue;
                }
                
                // 匹配原始名称
                match = nameOrigRe.match(line);
                if (match.hasMatch()) {
                    currentName = match.captured(1);
                }
            }
        }
        
        if (!skip && !currentName.isEmpty()) {
            lastValidName = currentName;
        }
    }

    // 处理最终的有效名称
    if (!lastValidName.isEmpty()) {
        appName = lastValidName;  // 直接赋值而不是使用<<
        return true;
    }
    
    // 回退到包名
    appName = packageName;
    return false;
}

QStringList aptssUpdater::getPackageIcons()
{
 
}
