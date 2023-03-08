#include "image_show.h"

#include <QHBoxLayout>
#include <QScreen> // Qt5 不再建议使用 QDesktopWidget
#include <QGuiApplication>

image_show::image_show(QWidget *parent) : QWidget(parent),
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

    // 获取当前app所在的主屏幕尺寸
    desktop_w = QGuiApplication::screenAt(QCursor().pos())->geometry().width();
    desktop_h = QGuiApplication::screenAt(QCursor().pos())->geometry().height();

    if (screen0.width() > int(desktop_w / 3 * 2) || screen0.height() > int(desktop_h / 3 * 2))
    {
        re_screen1 = screen0.scaled(QSize(int(desktop_w / 3 * 2), int(desktop_h / 3 * 2)), Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
    m_dialog->show();

    // 识别主屏幕尺寸并设置 widget 大小
    m_dialog->setFixedSize(qMin(desktop_w, m_image.width() + int(desktop_h / 16 * m_image.width() / m_image.height())), qMin(desktop_h, m_image.height() + int(desktop_h / 16)));

    m_dialog->move(QGuiApplication::screenAt(QCursor().pos())->geometry().center() - m_dialog->rect().center());
}
image_show::~image_show()
{
    delete m_dialog;
    delete m_label;
}
