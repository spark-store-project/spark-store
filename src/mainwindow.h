#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "aptssupdater.h"
#include "applistmodel.h"
#include "appdelegate.h"
#include <QListView>
#include <QJsonArray> // 添加头文件
#include <QScreen>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;  
private slots:
    // ⬅️ 声明一个槽函数，它将接收来自Updater的信号
    void handleUpdateFinished(bool success); 
private:
    Ui::MainWindow *ui;
    void checkUpdates();
    void initStyle();
    void runAptssUpgrade();
    AppListModel *m_model;
    AppDelegate *m_delegate;
    QListView *listView; // 声明 QListView 指针
    QJsonArray m_allApps; // 新增：保存所有应用数据
    void filterAppsByKeyword(const QString &keyword); // 新增：搜索过滤函数声明
};
#endif // MAINWINDOW_H
