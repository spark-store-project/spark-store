#include "webengineview.h"
#include "webenginepage.h"

WebEngineView::WebEngineView(QWidget *parent)
    : QWebEngineView(parent)
{
    setPage(new WebEnginePage(this));
}
