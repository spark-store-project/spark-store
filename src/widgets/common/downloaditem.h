#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H

#include <QWidget>
#include <QTextBrowser>
#include <QMenu>
#include <QAction>
#include "utils/utils.h"
#include <DDialog>
DWIDGET_USE_NAMESPACE

namespace Ui {
class DownloadItem;
}

class DownloadItem : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadItem(QWidget *parent = nullptr);
    ~DownloadItem();

    int num;
    bool free;
    static bool isInstall;
    bool reinstall;
    QString speed;
    QString out;
    QString pkgName;
    bool close;

    void setValue(qint64);
    void setMax(qint64);
    void setName(QString);
    QString getName();
    void readyInstall();

    void setFileName(QString);
    void seticon(const QPixmap);
    void closeDownload();
    void setSpeed(QString);

    void install(int);

private:
    Ui::DownloadItem *ui;

    QMenu *menu_install;
    QAction *action_dpkg;
    QAction *action_deepin;
    QAction *action_gdebi;

    DDialog *output_w;
    QTextBrowser *textbrowser;

private slots:
    void on_pushButton_install_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

signals:
    void finished();
};

#endif // DOWNLOADITEM_H
