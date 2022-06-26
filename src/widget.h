#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include <QJsonObject>
#include <QProcess>
#include <QFuture>
#include <QFutureWatcher>
#include <QToolButton>
#include <QTimer>
#include <QJsonArray>
#include <QFontDatabase>
#include <QMutex>

#include <DBlurEffectWidget>
#include <DTitlebar>
#include <DSearchEdit>
#include <DSettings>
#include <DSpinner>
#include <DWaterProgress>

#include "image_show.h"
#include "downloadlist.h"
#include "progressload.h"
#include "workerthreads.h"

#define LIST_MAX 99 // 一次最多下载数量
#define TMP_PATH "/tmp/spark-store"

DWIDGET_USE_NAMESPACE

namespace Ui {
class Widget;
}

class FlowLayout;
class DownloadController;

namespace AeaQt {
class HttpClient;
}

class Widget : public DBlurEffectWidget
{
    Q_OBJECT

public:
    explicit Widget(DBlurEffectWidget *parent = nullptr);
    ~Widget();

    void startRequest(QUrl url, QString fileName);
    void searchApp(QString);
    int nowDownload = 0;
    int allDownload = 0;
    int isdownload = false;
    void opensetting(); //打开设置页面
    void openUrl(QUrl);
    void setTheme(bool,QColor);
    DTitlebar* getTitlebar();

    static void sendNotification(const QString &message, const int msTimeout = 5000, const QString &icon = "spark-store");
    static void sendNotification(const char *message, const int msTimeout = 5000, const QString &icon = "spark-store");

private slots:

    void httpFinished();
    void httpReadyRead();
    void updateDataReadProgress(qint64,qint64);

    // SpkAppInfoLoaderThread的槽函数
    void sltAppinfoResetUi();
    void sltAppinfoTags(QStringList *tagList);
    void sltAppinfoDetails(QString *name, QString *details, QString *info,
                           QString *website, QString *packageName,
                           QUrl *fileUrl, int downloadCount, bool isInstalled,
                           bool isUpdated);
    void sltAppinfoIcon(QPixmap *icon);
    void sltAppinfoScreenshot(QPixmap *picture, int index);
    void sltAppinfoFinish();

    void displaySearchApp(QJsonArray array); // 展示搜索的APP信息
    void downloadIconsFinished(int arraysize); // 当前搜索列表图标是否下载完成

    void on_pushButton_download_clicked();
    void on_pushButton_return_clicked();
    void on_comboBox_server_currentIndexChanged(const QString &arg1);
    void on_pushButton_updateServer_clicked();
    void on_pushButton_updateApt_clicked();
    void on_pushButton_uninstall_clicked();
    void on_pushButton_clear_clicked();
    void on_pushButton_website_clicked();
    void on_pushButton_clicked();
    void on_btn_openDir_clicked();
    void on_stackedWidget_currentChanged(int arg1);
    void on_webEngineView_urlChanged(const QUrl &arg1);
    void on_webEngineView_loadStarted();
    void on_webEngineView_loadProgress(int progress);
    void on_webEngineView_loadFinished(bool arg1);
    void on_pushButton_refresh_clicked();
    void on_pushButton_update_clicked();

  public:
    QUrl url;

    downloadlist download_list[LIST_MAX];
    Ui::Widget *ui;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QList<QUrl> urList;
    QFile *file;
    QString appName;
    QString urladdress;
    QString pkgName;
    QString appweb;
    bool themeIsDark;

    QFont font;

private:
    void initUI();
    void initConfig();
    void chooseLeftMenu(int index);
    void setfoot(int);
    void updatefoot();
    void updateUI();

    quint64 dirFileSize(const QString &path);

private:
    QPushButton * left_list[15];
    QUrl menuUrl[13];
    ProgressLoad *m_loadweb;
    QLabel *m_loaderror = new QLabel;
    QString serverUrl;
    bool configCanSave = false;
    bool isBusy = false;
    int nowMenu = 0;    // 定位当前菜单
    int prePage = 0;    // 定位前一个页面
    long download_size = 0;
    long size1 = 0;
    long size2 = 0;
    QPixmap screen[5];
    QFuture<void> load;
    QFutureWatcher<void> watchScreenshotLoad;
    QTimer download_speed;
    QString type_name;
    QColor main_color;
    int foot;
    DSearchEdit *searchEdit = new DSearchEdit;
    DTitlebar *titlebar;
    DSpinner *spinner = new DSpinner;

    int count = 0;  // 记录当前搜索列表下载图标完成的个数
    QMutex mutex;   // 禁止多次搜索事件同时发生

    QList<image_show*> label_screen;
    SpkAppInfoLoaderThread appinfoLoadThread;

    AeaQt::HttpClient *httpClient;
    FlowLayout *applist_grid;
    QHBoxLayout *main;
    DownloadController *downloadController;
};

#endif // WIDGET_H
