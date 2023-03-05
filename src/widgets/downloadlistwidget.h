#ifndef DOWNLOADLISTWIDGET_H
#define DOWNLOADLISTWIDGET_H

#include <DBlurEffectWidget>

#include <QWidget>
#include <QTimer>

namespace Ui {
class DownloadListWidget;
}

DWIDGET_USE_NAMESPACE

class DownloadItem;
class DownloadController;
class DownloadListWidget : public DBlurEffectWidget
{
    Q_OBJECT

public:
    DownloadItem *addItem(QString name, QString fileName, QString pkgName, const QPixmap icon, QString downloadurl);
    int nowDownload = 0;
    int allDownload = 0;
    int toDownload = 0;
    QList<DownloadItem *> getDIList();
    QList<QUrl> getUrlList();
    void m_move(int x, int y);
    explicit DownloadListWidget(QWidget *parent = nullptr);
    ~DownloadListWidget() override;

    bool isDownloadInProcess();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    int isdownload = false;
    bool isBusy = false;
    QStringList dlist;
    QList<QUrl> urList;
    QList<DownloadItem *> downloaditemlist;
    DownloadController *downloadController = nullptr;
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

signals:
    void downloadFinished();
    void downloadProgress(int i);

private slots:
    void on_pushButton_clicked();
    void slotInstallFinished(bool success);
};

#endif // DOWNLOADLISTWIDGET_H
