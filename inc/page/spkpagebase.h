
#pragma once

#include <QScrollArea>
#include <spkresource.h>

class SpkPageBase : public QScrollArea
{
    Q_OBJECT
  public:
    SpkPageBase(QWidget *parent = nullptr);

  public slots:
    /**
     * @brief This signal is emitted by resource manager when a resource acquisition requested
     *        has finished.
     * @param id The request ID
     * @param result The data retrieved
     */
    virtual void ResourceAcquisitionFinished(int id, ResourceResult result);

    /**
     * @brief This is an optional signal for Resource Context objects, mainly used for notifying the
     *        context that it is now activated (therefore it needs to acquire the resource manager).
     */
    virtual void Activated();
};
