#ifndef MAINWINDOWDTK_H
#define MAINWINDOWDTK_H

#include <DMainWindow>
#include <DBlurEffectWidget>
#include <DTitlebar>
#include <DSearchEdit>
#include <QGraphicsDropShadowEffect>
#include <DGuiApplicationHelper>

#include <QPushButton>
#include <QDir>
#include <QDesktopServices>

#include "widgets/downloadlistwidget.h"
#include "utils/widgetanimation.h"
#include "dbus/dbussparkstoreservice.h"

DWIDGET_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public DBlurEffectWidget
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

private:
    QList<int> pageHistory;

    DownloadListWidget *downloadlistwidget;
    QPushButton *downloadButton;
    QPushButton *backButtom;
    DSearchEdit *searchEdit = new DSearchEdit;
    Ui::MainWindow *ui;

private slots:
    //接受来自dbus的url
    void onGetUrl(const QString &url);
    void on_pushButton_14_clicked();
};

#endif // MAINWINDOWDTK_H
