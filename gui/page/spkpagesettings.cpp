
#include "spkutils.h"
#include "page/spkpagesettings.h"
#include "spkmsgbox.h"

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

    CFG->BindField("url/repo", &this->mRepoListUrl,
                   "https://d.store.deepinos.org.cn/store/server.list");

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

    connect(ui->btnSave, &QPushButton::clicked,
            this, &SpkPageSettings::SaveConfiguration);
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
    ui->edtRepoListUrl->setText(CFG->ReadField("url/repo", "").toString());
    ui->cmbLightDarkTheme->setCurrentIndex(CFG->ReadField("ui/theme", 0).toInt());
  }

  void SpkPageSettings::SaveConfiguration()
  {
    auto ui = mSettingsUi;

    CFG->SetSettings("resource/concurrent", ui->spnConcurrentResDownloads->value());
    CFG->SetField("url/api", ui->edtApiUrl->text());
    CFG->SetField("url/res", ui->edtResourceUrl->text());
    CFG->SetSettings("dirs/cache", ui->edtResourceCachePath->text());
    CFG->SetField("dirs/download", ui->edtDownloadPath->text());
    CFG->SetSettings("internal/qss_path", ui->edtQssPath->text());
    CFG->SetField("url/repo", ui->edtRepoListUrl->text());

    if(!CFG->SetField("download/servers", ui->edtDownloadServers->toPlainText()))
      SpkMsgBox::StaticExec(tr("Cannot change distribution servers.\n"
                               "There's probably still downloads going on."),
                            tr("Cannot set distribution server"),
                            QMessageBox::Warning);

    if(!CFG->SetField("ui/theme", ui->cmbLightDarkTheme->currentIndex()))
      SpkMsgBox::StaticExec(tr("Auto mode can only be used when DDE plugin is loaded.\n"
                               "Option change is not applied."),
                            tr("Cannot set theme mode"),
                            QMessageBox::Warning);
  }

  void SpkPageSettings::Activated()
  {
    ReadConfiguration();
  }
}
