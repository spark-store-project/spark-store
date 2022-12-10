#ifndef APPINTOPAGE_H
#define APPINTOPAGE_H

#include <QWidget>
#include <QUrl>
#include <QString>
#include <QtConcurrent>
#include "backend/sparkapi.h"
#include "backend/image_show.h"
#include "widgets/downloadlistwidget.h"
namespace Ui {
class AppIntoPage;
}

class AppIntoPage : public QWidget
{
    Q_OBJECT

public:
    void clear();
    void setDownloadWidget(DownloadListWidget *w);
    void setTheme(bool dark);
    void openUrl(QUrl url);
    explicit AppIntoPage(QWidget *parent = nullptr);
    ~AppIntoPage();

private slots:
    void on_downloadButton_clicked();

    void on_pushButton_3_clicked();

private:
    QJsonObject info;
    QPixmap iconpixmap;
    QUrl spk;
    void isDownloading();
    void sltAppinfoTags(QStringList *tagList);
    DownloadListWidget *dw;
    Ui::AppIntoPage *ui;

signals:
    void clickedDownloadBtn();
};

#endif // APPINTOPAGE_H
