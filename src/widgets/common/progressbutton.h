#ifndef PROGRESSBUTTON_H
#define PROGRESSBUTTON_H

#include <QWidget>
#include <QTimer>
#include <QVariantAnimation>

class DownloadListWidget;
class ProgressButton : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressButton(QWidget *parent = nullptr);
    ~ProgressButton() override;

    void setIcon(QString svgPATH);
    void setBackgroundColor(QColor color);
    void setColor(QColor color);
    void setProgress(int progress);

    void setDownloadListWidget(DownloadListWidget *widget);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void startProcessing();
    void clicked();

private slots:
    void operationProcessing();

private:
    enum state
    {
        normal,
        hover,
        openProgress,
        closeProgress,
        recovery
    };

    DownloadListWidget *m_downloadListWidget = nullptr;

    state buttonState{state::normal};
    QColor backColor;
    QColor color;
    QString svgPath;
    int widthChangeValue{0};
    int progress{0};//处理百分比

    bool m_mouseMoved = false;
    bool m_isDownloadListWidgetVisible = true;
};

class WaterDrop : public QWidget
{
    Q_OBJECT

public:
    explicit WaterDrop(QWidget *parent = nullptr);
    void show();
    void move(const QPoint &point);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void onRadiusChanged(const QVariant &value);

private:
    QVariantAnimation *m_waterDropAnimation = nullptr;
    int m_animationRadius;
};

#endif // PROGRESSBUTTON_H
