
#include <QDesktopServices>

#include "page/spkpagehome.h"
#include "spkabout.h"
#include "gitver.h"

SpkUi::SpkPageHome::SpkPageHome(QWidget *parent) :
  SpkPageBase(parent)
{
  ui = new Ui::SpkHomepage;
  ui->setupUi(this);

  SetupUi();
}

void SpkUi::SpkPageHome::LinkActivated(QString s)
{
  QDesktopServices::openUrl(QUrl(s));
}

void SpkUi::SpkPageHome::SetupUi()
{
  ui->lblIcon->setPixmap(QIcon(":/icons/spark-store.svg").pixmap(QSize(128, 128)));

  ui->hlayTitle->setAlignment(Qt::AlignHCenter);

  ui->lblVersion->setText(ui->lblVersion->text().arg(GitVer::DescribeTags()));

  ui->lblNewAnnouncement->setVisible(false);

  // Click event will propagate to the main window and cause the window to move when
  // mouse enters the zoom & move detection area. Disable mouse event propagation.
  ui->lblAuthor->setAttribute(Qt::WA_NoMousePropagation, true);

  ui->widReloadCategory->setVisible(false);
  ui->widReloadCategory->setAttribute(Qt::WA_StyledBackground);
  ui->lblCategoryErrIcon->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(32, 32)));

  connect(ui->lblAuthor, &QLabel::linkActivated,
          this, &SpkPageHome::LinkActivated);
  connect(ui->btnSubmit, &QPushButton::clicked,
          [&](){ LinkActivated("https://upload.deepinos.org"); });
  connect(ui->btnFeedback, &QPushButton::clicked,
          [&](){ LinkActivated("https://www.deepinos.org/t/spark-feedback"); });
  connect(ui->btnDonation, &QPushButton::clicked,
          [&](){ LinkActivated("https://spark.deepinos.org.cn/"); });
  connect(ui->btnAbout, &QPushButton::clicked,
          [&](){ SpkAbout::Show(); });
}
