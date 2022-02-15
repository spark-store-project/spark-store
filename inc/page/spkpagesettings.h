
#pragma once

#include <QScrollArea>
#include <QVBoxLayout>
#include "page/spkpagebase.h"
#include "ui_settings.h"

namespace SpkUi
{
  class SpkPageSettings : public SpkPageBase
  {
      Q_OBJECT

    public:
      SpkPageSettings(QWidget *parent = nullptr);
      ~SpkPageSettings();

      void SetupUi();
      void ReadConfiguration();
      void SaveConfiguration();

      virtual void Activated() override;

    private slots:

    private:
      QScrollArea *mMainArea;
      QVBoxLayout *mMainLay;
      QWidget *mSettingsWidget;
      Ui::SpkUiSettings *mSettingsUi;

      QString mRepoListUrl;

  };
}
