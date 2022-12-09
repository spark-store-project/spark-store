#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QtConcurrent>
#include "../backend/sparkapi.h"
namespace Ui {
class SettingsPage;
}

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    void setTheme(bool dark);
    ~SettingsPage();

private slots:
    void on_pushButton_updateServer_clicked();

    void on_comboBox_server_currentIndexChanged(const QString &arg1);

private:
    bool configCanSave;
    void readServerList();
    void initConfig();
    Ui::SettingsPage *ui;
};

#endif // SETTINGSPAGE_H
