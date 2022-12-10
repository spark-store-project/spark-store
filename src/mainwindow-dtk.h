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
};

#endif // MAINWINDOWDTK_H
