
#include "spkutils.h"
#include "page/spkpagesettings.h"

namespace SpkUi
{
  SpkPageSettings::SpkPageSettings(QWidget *parent) :
    SpkPageBase(parent)
  {
    mMainArea = new QScrollArea();
    mMainLay = new QVBoxLayout(this);
    mSettingsWidget = new QWidget(this);
    mSettingsUi = new Ui::SpkUiSettings;

    mSettingsUi->setupUi(mSettingsWidget);

    mMainLay->addWidget(mMainArea);

    mMainArea->setWidget(mSettingsWidget);
    mMainArea->setWidgetResizable(true);

    SetupUi();
  }

  SpkPageSettings::~SpkPageSettings()
  {
    delete mSettingsWidget;
  }

  void SpkPageSettings::SetupUi()
  {
    auto ui = mSettingsUi;

    ui->lblSettingsTitle->setObjectName("styConfTitle");
    ui->lblCleanup->setObjectName("styConfTitle");
    ui->lblAdvanced->setObjectName("styConfTitle");
  }

  void SpkPageSettings::ReadConfiguration()
  {
    auto ui = mSettingsUi;

    ui->spnConcurrentResDownloads->setValue(CFG->ReadField("resource/concurrent", 5).toInt());
    ui->edtApiUrl->setText(CFG->ReadField("url/api", "").toString());
    ui->edtResourceUrl->setText(CFG->ReadField("url/res", "").toString());
    ui->edtResourceCachePath->setText(CFG->ReadField("dirs/cache", "").toString());
    ui->edtDownloadPath->setText(CFG->ReadField("dirs/download", "").toString());
    ui->edtDownloadServers->setPlainText(CFG->ReadField("download/servers", "").toString());
    ui->edtQssPath->setText(CFG->ReadField("internal/qss_path", "").toString());
  }

  void SpkPageSettings::SaveConfiguration()
  {

  }

  void SpkPageSettings::Activated()
  {
    ReadConfiguration();
  }

}
