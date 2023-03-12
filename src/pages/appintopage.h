#ifndef APPINTOPAGE_H
#define APPINTOPAGE_H

#include <QWidget>
#include <QJsonObject>
#include <QUrl>

namespace Ui {
class AppIntoPage;
}

class SparkAPI;
class DownloadListWidget;
class AppIntoPage : public QWidget
{
    Q_OBJECT

public:
    explicit AppIntoPage(QWidget *parent = nullptr);
    ~AppIntoPage() override;

    void openUrl(const QUrl &url);
    void clear();
    void setTheme(bool dark);
    void setDownloadWidget(DownloadListWidget *w);

private:
    void initUI();
    void initConnections();
    void isDownloading(const QUrl &url);
    void setAppinfoTags(const QStringList &tagList);
    void notifyUserUnsupportedTags(bool ubuntuSupport, bool deepinSupport, bool uosSupport);

signals:
    void clickedDownloadBtn();

private slots:
    void on_downloadButton_clicked();
    void on_pushButton_3_clicked();
    void on_shareButton_clicked();
    void on_updateButton_clicked();

private:
    Ui::AppIntoPage *ui;

    SparkAPI *api;
    DownloadListWidget *dw = nullptr;

    QJsonObject info;
    QPixmap iconpixmap;
    QUrl spk;
};

#endif // APPINTOPAGE_H
