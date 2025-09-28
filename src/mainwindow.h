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
    void updateButtonText(); // 新增：更新按钮文本
    
private slots:
    void handleUpdateFinished(bool success); // 新增：处理更新完成的槽函数
    void handleSelectionChanged(); // 新增：处理选择变化的槽函数
};
#endif // MAINWINDOW_H