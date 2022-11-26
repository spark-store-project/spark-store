#include "webenginepage.h"

#include <QDesktopServices>

WebEnginePage::WebEnginePage(QObject *parent)
    : QWebEnginePage(parent)
{
}

WebEnginePage::~WebEnginePage()
{
}

void WebEnginePage::setUrl(const QUrl &url)
{
    if (m_currentUrl == url) {
        return;
    }

    m_currentUrl = url;
    QWebEnginePage::setUrl(url);
}

QWebEnginePage *WebEnginePage::createWindow(QWebEnginePage::WebWindowType type)
{
    Q_UNUSED(type)

    WebEnginePage *page = new WebEnginePage(parent());
    connect(page, &WebEnginePage::urlChanged, this, &WebEnginePage::slotUrlChanged);
    return page;
}

void WebEnginePage::slotUrlChanged(const QUrl &url)
{
    if (m_currentUrl == url) {
        sender()->deleteLater();
        return;
    }

    qDebug() << Q_FUNC_INFO << m_currentUrl << url;

    QDesktopServices::openUrl(url);
    sender()->deleteLater();
}
