#include "applistpage.h"
#include "ui_applistpage.h"

AppListPage::AppListPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AppListPage)
{
    ui->setupUi(this);
    ui->webEngineView->page()->setBackgroundColor(Qt::transparent);
}
void AppListPage::setTheme(bool dark)
{
    isDark=dark;
    if(dark)
    {
        this->setStyleSheet("#frame{background-color: #252525;border-radius:14px;border:1px solid rgb(64, 64, 64);}");
    }else {
        //亮色模式
        this->setStyleSheet("#frame{background-color: #ffffff;border-radius:14px;border:1px solid rgb(229,229,229);}");
    }
    if(isSearch)
    {
        getSearchList(nowType);
    }else{
        getAppList(nowType);
    }
}
void AppListPage::getAppList(QString type)
{
    isSearch=false;
    nowType=type;
    SparkAPI *api=new SparkAPI(this);
    QString url;
    QString theme;
    if(isDark)
    {
        theme="theme=dark";
    }else{
        theme="theme=light";
    }
    if(type=="")
    {
        url=api->getServerUrl()+"store/#/new/?"+theme;
    }else{
        url=api->getServerUrl()+"store/#/new/applist?type="+type+"&"+theme;
    }

    ui->webEngineView->setUrl(url);
}

void AppListPage::getSearchList(QString keyword)
{
    isSearch=true;
    nowType=keyword;
    SparkAPI *api=new SparkAPI(this);
    QString url;
    QString theme;
    if(isDark)
    {
        theme="theme=dark";
    }else{
        theme="theme=light";
    }
    url=api->getServerUrl()+"store/#/new/search?keywords="+keyword+"&"+theme;
    ui->webEngineView->setUrl(url);
}

AppListPage::~AppListPage()
{
    delete ui;
}

void AppListPage::on_webEngineView_urlChanged(const QUrl &arg1)
{
    if(arg1.path().right(8) == "app.json")
        {
            QString url = arg1.toString();
            url = url.mid(url.indexOf("/store/"));
            url = "spk:/"+url;
            url = url.mid(0,url.indexOf("/app.json"));
            qDebug() << "程序跳转链接地址：" << url;
            ui->webEngineView->back();
            emit clicked(url);
        }
}

