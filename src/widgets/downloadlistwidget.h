#ifndef DOWNLOADLISTWIDGET_H
#define DOWNLOADLISTWIDGET_H

#include <QWidget>
#include <QTimer>
#include <DBlurEffectWidget>
#include <QNetworkAccessManager>
#include <QDesktopServices>
#include "widgets/common/downloaditem.h"
#include "backend/sparkapi.h"
#include "backend/downloadworker.h"

DWIDGET_USE_NAMESPACE
namespace Ui {
class DownloadListWidget;
}

class DownloadListWidget : public DBlurEffectWidget
{
    Q_OBJECT

public:
    void addItem(QString name, QString fileName, QString pkgName, const QPixmap icon, QString downloadurl);
    int isDownloading(QString url);
    void m_move(int x, int y);
    explicit DownloadListWidget(QWidget *parent = nullptr);
    ~DownloadListWidget();

private:
    int nowDownload = 0;
    int allDownload = 0;
    int isdownload = false;
    bool isBusy = false;
    QStringList dlist;
    QList<QUrl> urList;
    QList<DownloadItem *> downloaditemlist;
    DownloadController *downloadController;
    int nowdownload = 0;
    QString theSpeed;
    QTimer download_speed;
    long download_size = 0;
    long size1 = 0;
    long size2 = 0;
    void startRequest(QUrl url, QString fileName);
    void httpFinished();
    void updateDataReadProgress(QString speedInfo, qint64 bytesRead, qint64 totalBytes);
    void clearItem();
    QRect m_rect;
    Ui::DownloadListWidget *ui;
private slots:
    bool eventFilter(QObject *, QEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void on_pushButton_clicked();
};

#endif // DOWNLOADLISTWIDGET_H
