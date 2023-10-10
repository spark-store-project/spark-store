#include "webenginepage.h"

#include <QDesktopServices>
#include <QWebEngineSettings>
#include <QWebEngineProfile>

WebEnginePage::WebEnginePage(QObject *parent)
    : QWebEnginePage(parent)
{
    initHttpAcceptLanguage();
}

WebEnginePage::~WebEnginePage()
{
}

void WebEnginePage::setUrl(const QUrl &url)
{
    if (m_currentUrl == url)
    {
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

void WebEnginePage::initHttpAcceptLanguage()
{
    switch (QLocale::system().language()) {
    case QLocale::Chinese: {
        // 系统语言是中文，获取网页为中文 @momen @uniartisan
        profile()->setHttpAcceptLanguage("zh-CN,zh;q=0.8,en;q=0.6");
    } break;
    default:
        break;
    }
}

void WebEnginePage::slotUrlChanged(const QUrl &url)
{
    if (m_currentUrl == url)
    {
        sender()->deleteLater();
        return;
    }

    qDebug() << Q_FUNC_INFO << m_currentUrl << url;

    QDesktopServices::openUrl(url);
    sender()->deleteLater();
}
