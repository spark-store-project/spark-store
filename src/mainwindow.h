#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "aptssupdater.h"
#include "applistmodel.h"
#include "appdelegate.h"
#include <QListView>

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

private:
    Ui::MainWindow *ui;
    void checkUpdates();
    void initStyle();
    AppListModel *m_model;
    AppDelegate *m_delegate;
    QListView *listView; // 声明 QListView 指针
};
#endif // MAINWINDOW_H
