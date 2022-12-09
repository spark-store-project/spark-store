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

    }else {
        //亮色模式
        this->setStyleSheet("#frame{background-color: #ffffff;border-radius:14px;border:1px solid rgb(229,229,229);}#webEngineView{border-radius:14px;}");
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
    if(isDark)
    {
        url=api->getServerUrl()+"store/#/dark"+type;
    }else{
        url=api->getServerUrl()+"store/#/"+type;
    }
    ui->webEngineView->setUrl(url);
}

void AppListPage::getSearchList(QString keyword)
{
    isSearch=true;
    nowType=keyword;
    SparkAPI *api=new SparkAPI(this);
    QString url;
    if(isDark)
    {
        url=api->getServerUrl()+"store/#/darksearch?keywords="+keyword;
    }else{
        url=api->getServerUrl()+"store/#/search?keywords="+keyword;
    }
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

