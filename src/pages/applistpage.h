#ifndef APPLISTPAGE_H
#define APPLISTPAGE_H
#include <QMutex>
#include <QWidget>
#include <QListWidgetItem>
#include "backend/sparkapi.h"
namespace Ui {
class AppListPage;
}

class AppListPage : public QWidget
{
    Q_OBJECT

public:
    void setTheme(bool dark);
    void getSearchList(QString keyword);
    void getAppList(QString type);
    explicit AppListPage(QWidget *parent = nullptr);
    ~AppListPage();

private:
    QMutex mutex; // 禁止多次搜索事件同时发生
    bool isDark;
    bool isSearch;
    QString nowType;
    Ui::AppListPage *ui;
signals:
    void clicked(QUrl spk);
private slots:
    void on_webEngineView_urlChanged(const QUrl &arg1);
};

#endif // APPLISTPAGE_H
