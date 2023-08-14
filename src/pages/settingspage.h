#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QUrl>

namespace Ui {
class SettingsPage;
}

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    static void setIsDownload(bool isdownload);
    void updateUI();
    explicit SettingsPage(QWidget *parent = nullptr);
    void setTheme(bool dark);
    ~SettingsPage();

private slots:
    void on_pushButton_updateServer_clicked();

    void on_comboBox_server_currentIndexChanged(const QString &arg1);

    void on_pushButton_updateApt_clicked();

    void on_pushButton_clear_clicked();

    void on_pushButton_clearWebCache_clicked();

    void on_checkBox_clicked(bool checked);

public:
    static bool needUncompatibleNotification;

private:
    static bool isdownload;
    bool configCanSave;
    quint64 dirFileSize(const QString &path);
    void readServerList();
    void initConfig();
    Ui::SettingsPage *ui;

signals:
    void openUrl(QUrl spk);
};

#endif // SETTINGSPAGE_H
