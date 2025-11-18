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
    QString command = R"(env LANGUAGE=en_US /usr/bin/apt -c /opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf list --upgradable -o Dir::Etc::sourcelist="/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/aptss.list" -o Dir::Etc::sourceparts="/dev/null" -o APT::Get::List-Cleanup="0" | awk 'NR>1')";
    
    process.start("bash", QStringList() << "-c" << command);
    if (!process.waitForFinished(30000)) { // 30秒超时
        qWarning() << "Process failed to finish within 30 seconds.";
        process.kill();
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
    QStringList packageDetails;

    // 获取可更新包名列表
    QStringList updateablePackages;
    for (const QString &pkgInfo : packageName) {
        updateablePackages << pkgInfo.section(":", 0, 0).trimmed();
    }

    foreach (const QString &packageName, updateablePackages) {
        QProcess process;  // 在循环内部创建新的QProcess实例
        
        // 构建新命令（包含包名参数）
        QString command = QString("/usr/bin/apt download %1 --print-uris -c /opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf "
                                  "-o Dir::Etc::sourcelist=\"/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/aptss.list\" "
                                  "-o Dir::Etc::sourceparts=\"/dev/null\"").arg(packageName);

        process.start("bash", QStringList() << "-c" << command);
        if (!process.waitForFinished(30000)) { // 30秒超时
            qWarning() << "获取包信息失败：" << packageName << "（超时）";
            process.kill();
            continue;
        }

        QString output = process.readAllStandardOutput();
        // 使用正则匹配所有信息
        // 调整正则表达式匹配分组
        QRegularExpression regex(R"('([^']+)'\s+(\S+)\s+(\d+)\s+SHA512:([^\s]+))");  // 分组1:URL 分组2:文件名 分组3:大小 分组4:SHA512
        QRegularExpressionMatch match = regex.match(output);

        if (match.hasMatch()) {
            QString url = match.captured(1);
            QString fileName = match.captured(2);
            QString size = match.captured(3);
            QString sha512 = match.captured(4);
            
            // 调整字段顺序：包名 | 大小 | URL | SHA512
            packageDetails << QString("%1: %2|%3|%4").arg(packageName, size, url, sha512);
        }
    }

    qDebug() << "完整包信息：" << packageDetails;
    return packageDetails;
}





QStringList aptssUpdater::getDesktopAppNames()
{
    QStringList appNames;
    
    // 获取当前系统语言环境
    QString lang = QLocale().name().replace("_", "-");

    // 遍历所有可更新包（复用已有的临时文件数据）
    QStringList packages = packageName;
    
    foreach (const QString &package, packages) {
        QProcess dpkgProcess;  // 在循环内部创建新的QProcess实例
        
        QString packageName = package.split(":")[0];
        QString finalName = packageName; // 默认使用包名
        
        // 获取包文件列表
        dpkgProcess.start("dpkg", QStringList() << "-L" << packageName);
        if (!dpkgProcess.waitForFinished(30000)) { // 30秒超时
            qWarning() << "获取包文件列表失败：" << packageName << "（超时）";
            dpkgProcess.kill();
            continue;
        }
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
    QStringList packageIcons;
    
    // 遍历所有可更新包
    QStringList packages = packageName;
    
    foreach (const QString &package, packages) {
        QProcess dpkgProcess;  // 在循环内部创建新的QProcess实例
        
        QString packageName = package.split(":")[0];
        QString iconPath = ":/resources/default_icon.png"; // 默认图标
        
        // 获取包文件列表
        dpkgProcess.start("dpkg", QStringList() << "-L" << packageName);
        if (!dpkgProcess.waitForFinished(30000)) { // 30秒超时
            qWarning() << "获取包文件列表失败：" << packageName << "（超时）";
            dpkgProcess.kill();
            packageIcons << QString("%1: %2").arg(packageName, iconPath);
            continue;
        }
        QStringList files = QString(dpkgProcess.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);
        
        // 查找.desktop文件
        QStringList desktopFiles = files.filter(QRegularExpression("/(usr/share|opt/apps)/.*\\.desktop$"));
        
        // 从.desktop文件中提取图标
        foreach (const QString &desktopFile, desktopFiles) {
            QFile file(desktopFile);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QString line = in.readLine().trimmed();
                    if (line.startsWith("Icon=")) {
                        QString iconName = line.mid(5).trimmed();
                        
                        // 处理相对图标名（如Icon=vscode）
                        if (!iconName.contains('/')) {
                            // 查找标准图标路径
                            QStringList iconPaths = {
                                QString("/usr/share/pixmaps/%1.png").arg(iconName),
                                QString("/usr/share/icons/hicolor/48x48/apps/%1.png").arg(iconName),
                                QString("/usr/share/icons/hicolor/scalable/apps/%1.svg").arg(iconName),
                                QString("/opt/apps/%1/entries/icons/hicolor/48x48/apps/%2.png").arg(packageName, iconName)
                            };
                            
                            foreach (const QString &path, iconPaths) {
                                if (QFile::exists(path)) {
                                    iconPath = path;
                                    qDebug() << "找到图标文件:" << path;
                                    break;
                                }
                            }
                        } else {
                            // 已经是绝对路径
                            if (QFile::exists(iconName)) {
                                iconPath = iconName;
                                qDebug() << "使用绝对路径图标文件:" << iconName;
                            }
                        }
                        break;
                    }
                }
                file.close();
            }
        }
        
        // 如果.desktop中没有找到图标，尝试直接查找包中的图标文件
        if (iconPath == ":/resources/default_icon.png") {
            qDebug() << "未在.desktop文件中找到图标，尝试直接查找包中的图标文件";
            QStringList iconFiles = files.filter(QRegularExpression("/(usr/share/pixmaps|usr/share/icons|opt/apps/.*/entries/icons)/.*\\.(png|svg)$"));
            if (!iconFiles.isEmpty()) {
                iconPath = iconFiles.first();
                qDebug() << "从包中找到图标文件:" << iconPath;
            } else {
                qDebug() << "未在包中找到图标文件，使用默认图标";
            }
        }
        
        qDebug() << "包名:" << packageName << "图标路径:" << iconPath;
        packageIcons << QString("%1: %2").arg(packageName, iconPath);
    }
    
    return packageIcons;
}


QJsonArray aptssUpdater::getUpdateInfoAsJson()
{
    QJsonArray jsonArray;
    
    // 获取所有需要的信息
    QStringList sizes = getPackageSizes();
    QStringList names = getDesktopAppNames();
    QStringList icons = getPackageIcons();
    
    // 创建包名到各种信息的映射
    QHash<QString, QHash<QString, QString>> packageInfo;
    
    // 解析包版本信息
    for (const QString &pkg : packageName) {
        QStringList parts = pkg.split(": ");
        if (parts.size() >= 2) {
            QString packageName = parts[0];
            QStringList versions = parts[1].split(" → ");
            if (versions.size() == 2) {
                packageInfo[packageName]["current_version"] = versions[0];
                packageInfo[packageName]["new_version"] = versions[1];
            }
        }
    }
    
    // 解析包详细信息（新增部分）
    for (const QString &sizeInfo : sizes) {
        QStringList parts = sizeInfo.split(": ");
        if (parts.size() == 2) {
            QString packageName = parts[0];
            QStringList details = parts[1].split("|");
            if (details.size() == 3) {  // 现在包含大小|URL|SHA512
                packageInfo[packageName]["size"] = details[0];
                packageInfo[packageName]["url"] = details[1];
                packageInfo[packageName]["sha512"] = details[2];
            }
        }
    }
    
    // 解析应用名称信息
    for (const QString &nameInfo : names) {
        if (nameInfo.startsWith("[") && nameInfo.endsWith("]")) {
            QString content = nameInfo.mid(1, nameInfo.length() - 2);
            QStringList parts = content.split("|");
            if (parts.size() == 2) {
                QString displayName = parts[0];
                QString packageName = parts[1];
                packageInfo[packageName]["display_name"] = displayName;
            }
        }
    }
    
    // 解析图标信息
    for (const QString &iconInfo : icons) {
        QStringList parts = iconInfo.split(": ");
        if (parts.size() == 2) {
            QString packageName = parts[0];
            packageInfo[packageName]["icon"] = parts[1].trimmed();
        }
    }
    
    // 构建JSON数组
    for (const QString &packageName : packageInfo.keys()) {
        QJsonObject jsonObj;
        jsonObj["package"] = packageName;
        
        // 使用显示名称(如果有)，否则使用包名
        if (packageInfo[packageName].contains("display_name")) {
            jsonObj["name"] = packageInfo[packageName]["display_name"];
        } else {
            jsonObj["name"] = packageName;
        }
        
        jsonObj["current_version"] = packageInfo[packageName]["current_version"];
        jsonObj["new_version"] = packageInfo[packageName]["new_version"];
        jsonObj["icon"] = packageInfo[packageName]["icon"];
        jsonObj["ignored"] = false; // 默认不忽略
        
        // 如果有大小信息也加入
        if (packageInfo[packageName].contains("size")) {
            jsonObj["size"] = packageInfo[packageName]["size"];
        }
        
        // 在构建JSON对象时添加新字段（在jsonObj中添加）：
        if (packageInfo[packageName].contains("url")) {
            jsonObj["download_url"] = packageInfo[packageName]["url"];
            qDebug() << "生成的下载 URL:" << packageInfo[packageName]["url"]; // 检查生成的 URL
        } else {
            qWarning() << "未找到下载 URL，包名:" << packageName;
            jsonObj["download_url"] = ""; // 设置为空字符串以避免崩溃
        }
        jsonObj["sha512"] = packageInfo[packageName]["sha512"];
        jsonArray.append(jsonObj);
    }
    qDebug()<<jsonArray;
    return jsonArray;
}
