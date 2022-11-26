#include "webengineview.h"
#include "webenginepage.h"

#include <QDesktopServices>

WebEngineView::WebEngineView(QWidget *parent)
    : QWebEngineView(parent)
{
    setPage(new WebEnginePage(this));
}
