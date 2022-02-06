
#include <QDesktopServices>
#include "spkabout.h"
#include "gitver.h"

SpkAbout::SpkAbout(QWidget *parent) : SpkDialog(parent)
{
  setWindowModality(Qt::ApplicationModal);

//  mDialogWidget->setMaximumWidth(600);
  mDialogWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  setFixedSize(550, 450);
  SetResizable(false); // Do you like the dilemma of using self created widget?

  mIconLay = new QHBoxLayout;

  mSpkVersion = new QLabel;
  mSpkVersion->setText(tr("<h1>Spark Store</h1>"
                          "<h3>Version <a href=\"https://www.spark-app.store\">%1</a></h3>"
                          "<t>Built on %2 %3</t>")
                           .arg(GitVer::DescribeTags(),
                                __DATE__, __TIME__));
  connect(mSpkVersion, &QLabel::linkActivated,
          [](const QString &link){ QDesktopServices::openUrl(link); });

  mSpkIcon = new QLabel;
  mSpkIcon->setPixmap(QIcon(":/icons/spark-store.svg").pixmap(QSize(128, 128)));

  auto description = tr(
      "Spark Store was started when Chinese home-grown Linux operating systems "
      "had initially hit the market. Because the Linux desktop ecosystem is not "
      "good enough at the time being, volunteers built this small App Store in "
      "the hope that users can get useful applications faster.\n\n"
      "Right now we are not just a Chinese group. We are discovering our way into "
      "more Debian-based Linux OSes, and build a real community software repository "
      "for users around the world.");
  mDescriptionText = new QLabel;
  mDescriptionText->setObjectName("spk_about_desc");
  mDescriptionText->setWordWrap(true);
  mDescriptionText->setText(description);

  mIconLay->addStretch(3);
  mIconLay->addWidget(mSpkIcon);
  mIconLay->addStretch(1);
  mIconLay->addWidget(mSpkVersion);
  mIconLay->addStretch(3);
  mIconLay->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

  AddStretch();
  AddLayout(mIconLay);
  AddSpacing(18);
  AddWidget(mDescriptionText);
  AddStretch();
  SetMargin(18, 0, 18, 18);

  GetTitleBar()->SetOperationButton(SpkTitleBar::OperationButton::Close);
  GetTitleBar()->SetTitle(tr("About Spark Store"));
}

SpkAbout::~SpkAbout()
{
  //  delete mIconLay;
  //  delete mDescriptionText;
}

void SpkAbout::Show()
{
  SpkAbout *b = new SpkAbout;

  b->Exec();
  delete b;
}
