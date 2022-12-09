#ifndef WEBENGINEPAGE_H
#define WEBENGINEPAGE_H

#include <QWebEnginePage>

class WebEnginePage : public QWebEnginePage
{
    Q_OBJECT

public:
    explicit WebEnginePage(QObject *parent = nullptr);
    ~WebEnginePage() override;

    void setUrl(const QUrl &url);

protected:
    QWebEnginePage *createWindow(WebWindowType type) override;

private slots:
    void slotUrlChanged(const QUrl &url);

private:
    QUrl m_currentUrl;
};

#endif // WEBENGINEPAGE_H
