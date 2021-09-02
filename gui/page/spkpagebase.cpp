
#include "page/spkpagebase.h"

SpkPageBase::SpkPageBase(QWidget *parent) : QScrollArea(parent)
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
