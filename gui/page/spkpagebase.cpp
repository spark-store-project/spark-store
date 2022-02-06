
#include "page/spkpagebase.h"

/*
 * Documentation on
 * How to Add a New Page to Main Window Side Bar
 *
 * 1. Derive your page widget class from SpkPageBase (reference implementation
 *    in SpkPageSettings) and add them to CMakeLists
 *
 * 2. Add the ID for the page in enum SpkUi::SpkStackedPages (in spkmainwindow.h)
 *    and add it as a resource context if needed
 *
 * 3. Include the page's header file in spkmainwindow.h and add a pointer to it in
 *    SpkUi::SpkMainWidget
 *
 * 4. Add a tree item or icon button for the corresponding page inside
 *    SpkUi::SpkMainWidget, and initialize it in SpkUi::SpkMainWidget::SpkMainWidget.
 *    Take references of existing entries, and write similar code close to each other
 *    to make the source look nice. Don't forget to add the item to the UI.
 *
 * 5. Make the linkage between the page and the sidebar item at the end of
 *    SpkUi::SpkMainWidget::SpkMainWidget().
 *
 */

SpkPageBase::SpkPageBase(QWidget *parent) : QWidget(parent)
{

}

void SpkPageBase::ResourceAcquisitionFinished(int id, ResourceResult result)
{
  Q_UNUSED(id);
  Q_UNUSED(result);
}

void SpkPageBase::Activated()
{
  ; // Do nothing
}
