#include "progressbutton.h"
#include "widgets/downloadlistwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QMoveEvent>
#include <QSvgRenderer>

ProgressButton::ProgressButton(QWidget *parent)
    : QWidget{parent}
{
    svgPath = "";
    backColor = Qt::transparent;

    connect(this, &ProgressButton::startProcessing, this, &ProgressButton::operationProcessing, Qt::QueuedConnection);
}

ProgressButton::~ProgressButton()
{
}

void ProgressButton::setProgress(int progress)
{
    buttonState = state::openProgress;
    ProgressButton::progress = progress;
    if (progress == 0)
    {
        buttonState = state::normal;
        update();
    }
    if (progress == 100)
    {
        buttonState = state::closeProgress;
        update();
        WaterDrop *waterDrop = new WaterDrop(parentWidget());
        waterDrop->move(geometry().center());
        waterDrop->show();
    }
    repaint();
    update();
}

void ProgressButton::setDownloadListWidget(DownloadListWidget *widget)
{
    m_downloadListWidget = widget;
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

void ProgressButton::mousePressEvent(QMouseEvent *event)
{
    m_mouseMoved = false;
    m_isDownloadListWidgetVisible = m_downloadListWidget->isVisible();
    m_downloadListWidget->hide();
    QWidget::mousePressEvent(event);
}

void ProgressButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (buttonState == state::hover
        || buttonState == state::normal
        || buttonState == state::closeProgress)
    {
        update();
    }

    if (m_mouseMoved) {
        return QWidget::mouseReleaseEvent(event);
    }

    if (m_isDownloadListWidgetVisible) {
        m_downloadListWidget->hide();
    } else {
        QPoint pos(this->mapToGlobal(QPoint(0, 0)));
        pos += QPoint(width() / 2 - m_downloadListWidget->width() / 2, height() + 5);
        m_downloadListWidget->m_move(pos.x(), pos.y());
        m_downloadListWidget->setWindowState(windowState() & Qt::WindowState::WindowActive);
        m_downloadListWidget->activateWindow();
        m_downloadListWidget->show();
        m_downloadListWidget->raise();
    }
    m_isDownloadListWidgetVisible = m_downloadListWidget->isVisible();
    QWidget::mouseReleaseEvent(event);
}

void ProgressButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->accept();
}

void ProgressButton::mouseMoveEvent(QMouseEvent *event)
{
    m_mouseMoved = true;
    QWidget::mouseMoveEvent(event);
}

void ProgressButton::leaveEvent(QEvent *event)
{
    if (buttonState == state::hover)
    {
        buttonState = state::normal;
        update();
    }
    QWidget::leaveEvent(event);
}

void ProgressButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QRect rect = event->rect();

    if (buttonState == state::normal || buttonState == state::hover)
    {
        qreal radius = rect.height() / 2.0;
        painter.translate(QRectF(rect).center());
        painter.setPen(Qt::transparent);
        painter.setBrush(backColor);
        painter.drawEllipse(QPointF(0, 0), radius, radius);

        QSvgRenderer m_svgRender;
        m_svgRender.load(svgPath);
        m_svgRender.render(&painter, QRectF(-radius / 2, -radius / 2, radius, radius));
    }
    else if (buttonState == state::openProgress)
    {
        qreal radius = rect.height() / 2.0 - 1;
        painter.translate(QRectF(rect).center());
        painter.setPen(QPen(backColor.darker(), 2));
        painter.setBrush(backColor);
        painter.drawEllipse(QPointF(0, 0), radius, radius);

        QRectF rectF = QRectF(-radius, -radius, radius * 2, radius * 2);
        painter.setPen(QPen(color.darker(100), 2));
        qreal angle = progress * 360 / 100 * 1.0;
        painter.drawArc(rectF, 90 * 16, -qIntCast(angle * 16));

        QSvgRenderer m_svgRender;
        m_svgRender.load(svgPath);
        m_svgRender.render(&painter, QRectF(-radius / 2, -radius / 2, radius, radius));
    }
    else if (buttonState == state::closeProgress)
    {
        qreal radius = rect.height() / 2.0 - 1;
        painter.translate(QRectF(rect).center());
        painter.setPen(QPen(color.darker(100), 2));
        painter.setBrush(backColor);
        painter.drawEllipse(QPointF(0, 0), radius, radius);

        painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(-radius / 3, 0),
                         QPointF(-radius / 5, radius / 3));
        painter.drawLine(QPointF(-radius / 5, radius / 3),
                         QPointF(radius / 4, -radius / 4));
    }
    QWidget::paintEvent(event);
}

void ProgressButton::operationProcessing()
{
}

const int RADIUS = 60;
WaterDrop::WaterDrop(QWidget *parent)
    : QWidget(parent)
    , m_waterDropAnimation(new QVariantAnimation(this))
    , m_animationRadius(0)
{
    this->setFixedSize(QSize(RADIUS * 2, RADIUS * 2));
    // this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    // this->setAttribute(Qt::WA_TranslucentBackground);
    // this->setAttribute(Qt::WA_DeleteOnClose);

    // m_waterDropAnimation->setEasingCurve(QEasingCurve(static_cast<QEasingCurve::Type>(QRandomGenerator::global()->bounded(40))));
}

// 把鼠标点击的点转换为圆心点坐标
void WaterDrop::move(const QPoint &point)
{
    QPoint translatePoint = point - rect().center();
    QWidget::move(translatePoint);
}

void WaterDrop::show()
{
    m_waterDropAnimation->setStartValue(0);
    m_waterDropAnimation->setEndValue(RADIUS);
    m_waterDropAnimation->setDuration(350);

    connect(m_waterDropAnimation, &QVariantAnimation::valueChanged, this, &WaterDrop::onRadiusChanged);
    connect(m_waterDropAnimation, &QVariantAnimation::finished, this, &WaterDrop::deleteLater);
    m_waterDropAnimation->start(QVariantAnimation::DeleteWhenStopped);
    QWidget::show();
}

void WaterDrop::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(QBrush(QColor("#ffff80")), 5.0);
    pen.setWidth(5);
    painter.setPen(pen);
    painter.drawEllipse(event->rect().center(), m_animationRadius, m_animationRadius);

    QWidget::paintEvent(event);
}

void WaterDrop::onRadiusChanged(const QVariant &value)
{
    m_animationRadius = value.toInt();
    update();
}
