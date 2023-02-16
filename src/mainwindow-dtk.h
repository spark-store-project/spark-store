#ifndef MAINWINDOWDTK_H
#define MAINWINDOWDTK_H

#include "widgets/base/basewidgetopacity.h"

#include <DMainWindow>
#include <DTitlebar>
#include <DPushButton>
#include <DSearchEdit>
#include <DGuiApplicationHelper>

#include <QSystemTrayIcon>

DWIDGET_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class ProgressButton;
class DownloadListWidget;
class MainWindow : public BaseWidgetOpacity
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void openUrl(const QString &url);

private:
    void initUI();
    void initTitleBar();
    void initTitleBarMenu();
    void initLeftMenu();
    void initTrayIcon();
    void initConnections();
    void initDbus();
    void initTmpDir();
    void switchPage(int now);
    void updateUi(int now);

public slots:
    void notify(QObject *receiver, QEvent *event);

private slots:
    //接受来自dbus的url
    void onGetUrl(const QString &url);
    void onNewProcessInstance(qint64 pid, const QStringList &arguments);
    void on_pushButton_14_clicked();

private:
    Ui::MainWindow *ui;

    DPushButton *backButton;
    DSearchEdit *searchEdit;
    ProgressButton *downloadButton;
    DownloadListWidget *downloadlistwidget;

    QSystemTrayIcon *trayIcon = nullptr;

    QList<int> pageHistory;
};

#endif // MAINWINDOWDTK_H
