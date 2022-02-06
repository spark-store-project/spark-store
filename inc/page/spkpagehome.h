
#pragma once

#include <QVBoxLayout>
#include "page/spkpagebase.h"
#include "ui_homepage.h"

namespace SpkUi
{
  class SpkPageHome : public SpkPageBase
  {
      Q_OBJECT
    public:
      SpkPageHome(QWidget *parent = nullptr);
      ~SpkPageHome() { }

      Ui::SpkHomepage *ui;

    private slots:
      void LinkActivated(QString);

    private:
      void SetupUi();
  };
}
