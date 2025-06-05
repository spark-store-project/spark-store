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
