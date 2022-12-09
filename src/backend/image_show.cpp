#include "image_show.h"

#include <QHBoxLayout>
#include <QScreen>  // Qt5 不再建议使用 QDesktopWidget
#include <QGuiApplication>

image_show::image_show(QWidget *parent) :
    QWidget(parent),
    m_dialog(new big_image),
    m_label(new QLabel)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_label);
    setLayout(layout);
    m_label->setText("layout");
    m_label->setScaledContents(true);
}

void image_show::setImage(QPixmap image)
{
    QImage screen0;
    screen0 = image.toImage();
    QImage re_screen1;
    QImage re_screen0 = screen0.scaled(QSize(400, 300), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 获取主屏幕尺寸
    desktop_w = QGuiApplication::primaryScreen()->geometry().width();
    desktop_h = QGuiApplication::primaryScreen()->geometry().height();

    if(screen0.width() > (desktop_w - 20) || screen0.height() > (desktop_h - 20))
    {
        re_screen1 = screen0.scaled(QSize(desktop_w - 20, desktop_h - 20), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_image = QPixmap::fromImage(re_screen1);
    }
    else
    {
        m_image = image;
    }

    m_label->setPixmap(QPixmap::fromImage(re_screen0));
}

void image_show::mousePressEvent(QMouseEvent *)
{
    m_dialog->setimage(m_image);
    m_dialog->showFullScreen();

    // 识别主屏幕尺寸并设置 widget 大小
    m_dialog->setFixedSize(desktop_w, desktop_h);

    m_dialog->move(0,0);
}
image_show::~image_show()
{
    delete m_dialog;
    delete m_label;
}
