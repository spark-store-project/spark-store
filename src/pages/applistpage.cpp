#include "applistpage.h"
#include "ui_applistpage.h"

AppListPage::AppListPage(QWidget *parent) : QWidget(parent),
                                            ui(new Ui::AppListPage)
{
    ui->setupUi(this);
    ui->webEngineView->page()->setBackgroundColor(Qt::transparent);
}
void AppListPage::setTheme(bool dark)
{
    isDark = dark;
    if (dark)
    {
        this->setStyleSheet("#frame{background-color: #252525;border-radius:14px;border:1px solid rgb(64, 64, 64);}");
    }
    else
    {
        // 亮色模式
        this->setStyleSheet("#frame{background-color: #ffffff;border-radius:14px;border:1px solid rgb(229,229,229);}");
    }
    if (isSearch)
    {
        getSearchList(nowType);
    }
    else
    {
        getAppList(nowType);
    }
}
void AppListPage::getAppList(QString type)
{
    isSearch = false;
    nowType = type;
    SparkAPI *api = new SparkAPI(this);
    QString url;
    QString theme;
    if (isDark)
    {
        theme = "dark";
    }
    else
    {
        theme = "";
    }
    if (type == "")
    {
        url = api->getServerUrl() + "aarch64-store/#/"+ theme;
    }
    else
    {
        url = api->getServerUrl() + "aarch64-store/#/"+ theme + type;
    }

    ui->webEngineView->setUrl(url);
    delete api;
}

void AppListPage::getSearchList(const QString &keyword)
{
    isSearch = true;
    nowType = keyword;
    SparkAPI *api = new SparkAPI(this);
    QString url;
    QString theme;
    if (isDark)
    {
        theme = "theme=dark";
    }
    else
    {
        theme = "theme=light";
    }
    url = api->getServerUrl() + "store/#/flamescion/search?keywords=" + QUrl::toPercentEncoding(keyword) + "&" + theme;
    ui->webEngineView->setUrl(url);
    delete api;
}

AppListPage::~AppListPage()
{
    delete ui;
}

void AppListPage::on_webEngineView_urlChanged(const QUrl &arg1)
{
    if (arg1.path().right(8) == "app.json")
    {
        QString url = arg1.toString();
        url = url.mid(url.indexOf("/aarch64-store/"));
        url = "spk:/" + url;
        url = url.mid(0, url.indexOf("/app.json"));
        qDebug() << "程序跳转链接地址：" << url;
        ui->webEngineView->back();
        emit clicked(url);
    }
}
