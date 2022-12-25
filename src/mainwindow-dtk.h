#ifndef MAINWINDOWDTK_H
#define MAINWINDOWDTK_H

#include <DMainWindow>
#include <DTitlebar>
#include <DSearchEdit>
#include <QGraphicsDropShadowEffect>
#include <DGuiApplicationHelper>

#include <QPushButton>
#include <QDir>
#include <QDesktopServices>

#include "widgets/base/basewidgetopacity.h"
#include "widgets/downloadlistwidget.h"
#include "widgets/common/progressbutton.h"
#include "utils/widgetanimation.h"
#include "dbus/dbussparkstoreservice.h"

DWIDGET_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public BaseWidgetOpacity
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void openUrl(QUrl);

private:
    void initDbus();
    void initConfig();
    void switchPage(int now);
    void updateUi(int now);

private slots:
    //接受来自dbus的url
    void onGetUrl(const QString &url);
    void onNewProcessInstance(qint64 pid, const QStringList &arguments);
    void on_pushButton_14_clicked();

private:
    QList<int> pageHistory;

    Ui::MainWindow *ui;
    DownloadListWidget *downloadlistwidget;
    ProgressButton *downloadButton;
    QPushButton *backButtom;
    DSearchEdit *searchEdit;
};

#endif // MAINWINDOWDTK_H
