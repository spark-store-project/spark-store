
#include <QtConcurrent/QtConcurrentRun>
#include <QMutexLocker>
#include <QFutureWatcher>
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
    ui = new Ui::SpkUiSettings;

    ui->setupUi(mSettingsWidget);

    mMainLay->addWidget(mMainArea);

    mMainArea->setWidget(mSettingsWidget);
    mMainArea->setWidgetResizable(true);

    CFG->BindField("url/repo", &this->mRepoListUrl,
                   "https://d.store.deepinos.org.cn/store/server.list");

    mBytesDownloads = mBytesResource = -1;

    connect(&mFwResourceClean, &QFutureWatcher<void>::finished,
            this, &SpkPageSettings::CleanedResource);
    connect(&mFwResourceCount, &QFutureWatcher<void>::finished,
            this, &SpkPageSettings::CountFinishResource);
    connect(&mFwDownloadClean, &QFutureWatcher<void>::finished,
            this, &SpkPageSettings::CleanedDownload);
    connect(&mFwDownloadCount, &QFutureWatcher<void>::finished,
            this, &SpkPageSettings::CountFinishDownload);
    connect(ui->btnViewDownloadedContent, &QPushButton::clicked,
            this, &SpkPageSettings::on_btnViewDownloadedContent_clicked);
    connect(ui->btnViewResourceCache, &QPushButton::clicked,
            this, &SpkPageSettings::on_btnViewResourceCache_clicked);
    connect(ui->btnCleanDownloadedContent, &QPushButton::clicked,
            this, &SpkPageSettings::on_btnCleanDownloadedContent_clicked);
    connect(ui->btnCleanResourceCache, &QPushButton::clicked,
            this, &SpkPageSettings::on_btnCleanResourceCache_clicked);

    SetupUi();
  }

  SpkPageSettings::~SpkPageSettings()
  {
    delete mSettingsWidget;
  }

  void SpkPageSettings::SetupUi()
  {
    ui->lblSettingsTitle->setObjectName("styConfTitle");
    ui->lblCleanup->setObjectName("styConfTitle");
    ui->lblAdvanced->setObjectName("styConfTitle");

    connect(ui->btnSave, &QPushButton::clicked,
            this, &SpkPageSettings::SaveConfiguration);
  }

  void SpkPageSettings::ReadConfiguration()
  {
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

  void SpkPageSettings::CountCleaning()
  {
    ui->lblSizeDownloadedContent->setText(tr("Counting..."));
    ui->lblSizeResourceCache->setText(tr("Counting..."));
    auto futureDownload = QtConcurrent::run([&]()
      {
        QDirIterator itr(ui->edtDownloadPath->text().replace('*', QDir::homePath()),
                         QDirIterator::Subdirectories);
        if(mMutDownload.tryLock(0))
        {
          int64_t size = 0;
          while(itr.hasNext())
          {
            QFile f(itr.next());
            size += f.size();
          }
          mBytesDownloads = size;
          mMutDownload.unlock();
        }
      });
    auto futureResource = QtConcurrent::run([&]()
    {
      QDirIterator itr(ui->edtResourceCachePath->text().replace('*', QDir::homePath()),
                       QDirIterator::Subdirectories);
      if(mMutResource.tryLock((0)))
      {
        int64_t size = 0;
        while(itr.hasNext())
        {
          QFile f(itr.next());
          size += f.size();
        }
        mBytesResource = size;
        mMutResource.unlock();
      }
    });
    mFwDownloadCount.setFuture(futureDownload);
    mFwResourceCount.setFuture(futureResource);
  }

  void SpkPageSettings::CleanedResource()
  {
    ui->lblSizeResourceCache->setText(tr("Cleaned"));
  }

  void SpkPageSettings::CleanedDownload()
  {
    ui->lblSizeDownloadedContent->setText(tr("Cleaned"));
  }

  void SpkPageSettings::Activated()
  {
    ReadConfiguration();
    CountCleaning();
  }

  void SpkPageSettings::CountFinishResource()
  {
    if(mBytesResource >= 0)
      ui->lblSizeResourceCache->setText(SpkUtils::BytesToSize(mBytesResource));
  }

  void SpkPageSettings::CountFinishDownload()
  {
    if(mBytesDownloads >= 0)
      ui->lblSizeDownloadedContent->setText(SpkUtils::BytesToSize(mBytesDownloads));
  }

  void SpkPageSettings::on_btnViewResourceCache_clicked()
  {
    QDesktopServices::openUrl(ui->edtResourceCachePath->text().replace('*', QDir::homePath()));
  }

  void SpkPageSettings::on_btnViewDownloadedContent_clicked()
  {
    QDesktopServices::openUrl(ui->edtDownloadPath->text().replace('*', QDir::homePath()));
  }


void SpkPageSettings::on_btnCleanResourceCache_clicked()
{
  ui->lblSizeResourceCache->setText(tr("Cleaning..."));
  auto future = QtConcurrent::run([&]()
    {
      QDirIterator itr(ui->edtResourceCachePath->text().replace('*', QDir::homePath()),
                       QDirIterator::Subdirectories);
      if(mMutDownload.tryLock(0))
      {
        int64_t size = 0;
        while(itr.hasNext())
        {
          QFile f(itr.next());
          f.remove();
        }
        mBytesDownloads = size;
        mMutDownload.unlock();
      }
    });
  mFwResourceClean.setFuture(future);
}


void SpkPageSettings::on_btnCleanDownloadedContent_clicked()
{
  ui->lblSizeDownloadedContent->setText(tr("Cleaning..."));
  auto futureDownload = QtConcurrent::run([&]()
    {
      QDirIterator itr(ui->edtDownloadPath->text().replace('*', QDir::homePath()),
                       QDirIterator::Subdirectories);
      if(mMutDownload.tryLock(0))
      {
        int64_t size = 0;
        while(itr.hasNext())
        {
          QFile f(itr.next());
          f.remove();
        }
        mBytesDownloads = size;
        mMutDownload.unlock();
      }
    });
  mFwDownloadClean.setFuture(futureDownload);
}

}
