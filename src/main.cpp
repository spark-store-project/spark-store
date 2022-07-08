#include <DApplication>
#include <DApplicationSettings>
#include <DWidgetUtil>      // Dtk::Widget::moveToCenter(&w); 要调用它，就得引用 DWidgetUtil
#include <DAboutDialog>
#include <QVector>
#include <QScreen>

#include "widget.h"

DWIDGET_USE_NAMESPACE
int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();  // 已废弃，但是对于非deepin桌面可以正常使用标题栏
    DApplication::setAttribute(Qt::AA_EnableHighDpiScaling);    // 开启 Hidpi 支持
    // 程序内强制添加"-platformtheme deepin"参数喂给Qt让Qt正确使用Deepin主题修复各种奇怪样式问题
    QVector<char*> fakeArgs(argc + 2);
    fakeArgs[0] = argv[0];
    fakeArgs[1] = "-platformtheme";
    fakeArgs[2] = "deepin";
    for(int i = 1; i < argc; i++) fakeArgs[i + 2] = argv[i];
    int fakeArgc = argc + 2; // 为啥DApplication的argc要用引用啊？
    DApplication a(fakeArgc, fakeArgs.data());


    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.loadTranslator();     // 载入翻译

    //Customized DAboutDialog 
     
     DAboutDialog dialog;
      a.setAboutDialog(&dialog);
      dialog.setLicense(QObject::tr("We publish this program under GPL V3"));
      dialog.setVersion(DApplication::buildVersion("Version 3.1.3-1"));
      dialog.setProductIcon(QIcon::fromTheme("spark-store"));  // 设置Logo
      dialog.setProductName(QLabel::tr("Spark Store"));
      dialog.setDescription(
                  QObject::tr(
                      "<span style=' font-size:10pt;font-weight:60;'>An appstore powered by  community</span><br/>"
                      "<a href='https://www.spark-app.store/'>https://www.spark-app.store</a><br/>"
                      "<span style=' font-size:12pt;'>Spark developers</span>"
                      )
                  );
      dialog.setProductName(QLabel::tr("Spark Store"));
      dialog.setCompanyLogo(QPixmap(":/spark-store.png"));
      dialog.setWebsiteName(QObject::tr("The Spark Project"));
      dialog.setWebsiteLink("https://gitee.com/deepin-community-store");
     

    a.setProductName(QLabel::tr("Spark Store"));
    a.setProductIcon(QIcon::fromTheme("spark-store"));  //设置Logo
    a.setOrganizationName("spark-union");
    a.setOrganizationDomain("https://www.deepinos.org/");
    a.setApplicationName("Spark Store");    //不需要翻译，否则 ~/.local/share/ 下文件夹名称也被翻译为中文
    a.setApplicationVersion(DApplication::buildVersion("3.1.3-1"));
    a.setApplicationAcknowledgementPage("https://gitee.com/deepin-community-store/spark-store");
    a.setApplicationDescription(
                QObject::tr(
                    "<span style='font-size:10pt;font-weight:60;'>An appstore powered by deepin community</span><br/>"
                    "<a href='https://www.spark-app.store/'>https://www.spark-app.store</a><br/>"
                    "<span style='font-size:12pt;'>Spark developers</span><br/><br/>"
                    "Published under GPL V3"
                    )
                );

    // 限制单实例运行
    if(!a.setSingleInstance("spark-store"))
    {
        return -1;
    }

    // 保存窗口主题设置
    DApplicationSettings settings;

    Widget w;
    QScreen *s = DApplication::primaryScreen();
    int d_w = s->geometry().width();
    int d_h = s->geometry().height();

    if(d_w <= 1366)
    {
        w.setMinimumWidth(925);
        w.resize(925,650);
    }
    else
    {
        w.setMinimumWidth(1180);
        w.resize(1180,760);
    }

    if(d_h <= 768)
    {
        w.setMinimumHeight(650);
        w.resize(925,650);
    }
    else
    {
        w.setMinimumHeight(760);
        w.resize(1180,760);
    }

    QString arg1 = argv[1];
    if(arg1.left(6)=="spk://")
    {
        w.openUrl(QUrl(argv[1]));
    }

    // 让打开时界面显示在正中
    Dtk::Widget::moveToCenter(&w);

    w.show();

    return a.exec();
}
