
#include "gitver.h"
#include "spkabout.h"

SpkAbout::SpkAbout(QWidget *parent) : SpkDialog(parent)
{
  setWindowModality(Qt::ApplicationModal);

  mIconLay = new QHBoxLayout;

  mSpkVersion = new QLabel;
  mSpkVersion->setText(tr("<h1>Spark Store</h1>"
                          "<h3>Version %1</h3>"
                          "<t>Code committed on (%2)</t><br>"
                          "<t>Built on %3 %4</t>")
                       .arg(GitVer::DescribeTags())
                       .arg(GitVer::CommitDate())
                       .arg(__DATE__)
                       .arg(__TIME__));

  mSpkIcon = new QLabel;
  mSpkIcon->setPixmap(QIcon(":/icons/spark-store.svg").pixmap(QSize(128, 128)));

  QFile lipsum(":/info/lipsum.txt");
  lipsum.open(QFile::ReadOnly);
  mDescriptionText = new QLabel;
  mDescriptionText->setText(lipsum.readAll());
  lipsum.close();

  mIconLay->addWidget(mSpkIcon);
  mIconLay->addWidget(mSpkVersion);
  mIconLay->setAlignment(Qt::AlignVCenter);
  AddLayout(mIconLay);
  AddSpacing(18);
  AddWidget(mDescriptionText);
  SetMargin(18);

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
