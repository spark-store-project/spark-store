#ifndef PROGRESSBUTTON_H
#define PROGRESSBUTTON_H


#include <QtWidgets/QWidget>

#include <QPaintEvent>

#include <QTimer>
#include <QTimerEvent>
#include<QColor>
#include <QList>
#include <QMouseEvent>


class ProgressButton : public QWidget
{
    Q_OBJECT
public:
    ProgressButton(QWidget *parent = nullptr);
    void setIcon(QString svgPATH);
    void setBackgroundColor(QColor color);
    void setColor(QColor color);
    void setProgress(int progress);
    ~ProgressButton();
signals:
    void startProcessing();
    void clicked();

protected:
    void paintEvent(QPaintEvent *event)override;
    void leaveEvent(QEvent *event)override;
    void mousePressEvent(QMouseEvent *event)override;

private:
    enum class state
    {
        normal,
        hover,
        openProgress,
        closeProgress,
        recovery
    }
    buttonState{state::normal};
    QColor backColor;
    QColor color;
    QString svgPath;
    int widthChangeValue{0};
    void operationProcessing();
    int progress{0};//处理百分比
};

class WaterDrop : public QWidget
{
    Q_OBJECT

public:
    WaterDrop(QWidget *parent = Q_NULLPTR);
    void show();
    void move(const QPoint &point);

private:
    void paintEvent(QPaintEvent *event);
    void onRadiusChanged(QVariant value);

private:
    class QVariantAnimation* m_waterDropAnimation;
    int m_animationRadius;
};



#endif // PROGRESSBUTTON_H
