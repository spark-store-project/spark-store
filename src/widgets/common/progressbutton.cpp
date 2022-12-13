#include "progressbutton.h"
#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QConicalGradient>
#include <QVariantAnimation>
#include <QSvgRenderer>
ProgressButton::ProgressButton(QWidget *parent)
    : QWidget{parent}
{
    //this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    setMinimumWidth(36);
    setMinimumHeight(36);
    svgPath="";
    backColor=Qt::transparent;
    setMouseTracking(true);

    connect(this,&ProgressButton::startProcessing,this,&ProgressButton::operationProcessing,Qt::QueuedConnection);
}

ProgressButton::~ProgressButton()
{
}

void ProgressButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);
    auto rect = event->rect();

    if(buttonState == state::normal || buttonState == state::hover)
    {
        auto radiu = (rect.height() - 6) / 2;
        painter.translate(rect.center());
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor(buttonState == state::normal ? color : color.darker()));
        //painter.drawEllipse(QPoint(0,0),radiu,radiu);

        //radiu -= 3;
        painter.setBrush(backColor);
        painter.drawEllipse(QPoint(0,0),radiu,radiu);

        QSvgRenderer m_svgRender;
        m_svgRender.load(svgPath);
        m_svgRender.render(&painter ,QRectF(-radiu / 2,-radiu / 2,radiu,radiu));
    }
    else if(buttonState == state::openProgress)
    {
        painter.translate(rect.center());
        auto radiu = (rect.height() - 6) / 2 -3;
        painter.setBrush(backColor);
        painter.setPen(QPen(backColor,3));
        painter.drawEllipse(QPoint(0,0),radiu,radiu);

        painter.setPen(QPen(backColor,3));

        QSvgRenderer m_svgRender;
        m_svgRender.load(svgPath);
        m_svgRender.render(&painter ,QRectF(-radiu / 2,-radiu / 2,radiu,radiu));

        QRect rect = QRect(-radiu, -radiu,
                           radiu*2, radiu*2);

        painter.setPen(QPen(color.darker(100),3));
        auto angle = progress * 360 / 100;
        painter.drawArc(rect.adjusted(-3,-3,3,3),90 * 16,-static_cast<int>(angle * 16));
    }
    else if(buttonState == state::closeProgress)
    {
        auto radiu = (rect.height() - 6) / 2;
        painter.translate(rect.center());
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor(0, 0, 0, 63));
        painter.drawEllipse(QPoint(0,0),radiu,radiu);

        radiu -= 3;
        painter.setBrush(backColor);
        painter.drawEllipse(QPoint(0,0),radiu,radiu);

        painter.setPen(QPen(color,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        painter.drawLine(QPoint(-radiu / 3,0),
                         QPoint(-radiu / 5,radiu / 3));
        painter.drawLine(QPoint(-radiu / 5,radiu / 3),
                         QPoint(radiu / 4,-radiu / 4));
    }
}

void ProgressButton::setProgress(int progress)
{
    buttonState = state::openProgress;
    ProgressButton::progress = progress;
    if(progress == 0)
    {
        buttonState = state::normal;
        update();
    }
    if(progress == 100)
    {
        buttonState = state::closeProgress;
        update();
        auto waterDrop = new WaterDrop();
        waterDrop->move(this->mapToGlobal(this->rect().center()));
        waterDrop->show();
    }
    repaint();
    update();
}

void ProgressButton::setIcon(QString svgPATH)
{
    svgPath = svgPATH;
    update();
}

void ProgressButton::setBackgroundColor(QColor color)
{
    backColor = color;
    update();
}

void ProgressButton::setColor(QColor color)
{
    ProgressButton::color = color;
    update();
}

void ProgressButton::leaveEvent(QEvent *event)
{
    if(buttonState == state::hover)
    {
        buttonState = state::normal;
        update();
    }
    QWidget::leaveEvent(event);
}

void ProgressButton::mousePressEvent(QMouseEvent *event)
{
    if(buttonState == state::hover || buttonState == state::normal)
    {
        widthChangeValue = (this->width() - 6) / 2;
        update();
    }
    else if(buttonState == state::closeProgress)
    {
        update();
    }
    emit clicked();
    QWidget::mousePressEvent(event);
}

void ProgressButton::operationProcessing()
{

}

const int RADIUS = 60;
WaterDrop::WaterDrop(QWidget *parent)
    : QWidget(parent)
    , m_waterDropAnimation(nullptr)
    , m_animationRadius(0)
{
    this->setFixedSize(QSize(RADIUS * 2, RADIUS * 2));
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setAttribute(Qt::WA_DeleteOnClose);
    m_waterDropAnimation = new QVariantAnimation(this);
//    m_waterDropAnimation->setEasingCurve(QEasingCurve(static_cast<QEasingCurve::Type>(QRandomGenerator::global()->bounded(40))));
}

//把鼠标点击的点转换为圆心点坐标
void WaterDrop::move(const QPoint &point)
{
    QPoint translatePoint = point - QPoint(RADIUS, RADIUS);
    QWidget::move(translatePoint);
}

void WaterDrop::show()
{
    m_waterDropAnimation->setStartValue(0);
    m_waterDropAnimation->setEndValue(RADIUS);
    m_waterDropAnimation->setDuration(350);

    connect(m_waterDropAnimation, &QVariantAnimation::valueChanged, this, &WaterDrop::onRaduisChanged);
    connect(m_waterDropAnimation, &QVariantAnimation::finished, this, &WaterDrop::close);
    m_waterDropAnimation->start();
    QWidget::show();
}

void WaterDrop::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setColor(QColor(0xffffff80));
    pen.setWidth(5);
    painter.setPen(pen);
    painter.drawEllipse(event->rect().center(),m_animationRadius, m_animationRadius);
}

void WaterDrop::onRaduisChanged(QVariant value)
{
    m_animationRadius = value.toInt();
    update();
}
