
#include <QApplication>
#include "spkabout.h"
#include "inc/page/spkpageuitest.h"
#include "spkpopup.h"
#include "spkui_general.h"

SpkUi::SpkPageUiTest::SpkPageUiTest(QWidget *parent) : QSplitter(parent)
{
  setObjectName("spk_pg_qsstest");

  TextStylesheet = new QTextEdit(this);
  TextStylesheet->setObjectName("spk_pg_qsstest_qsstext");
  TextStylesheet->setPlainText(SpkUi::CurrentStylesheet);

  BtnApply = new QPushButton(this);
  BtnApply->setObjectName("spk_pg_qsstest_btnapply");
  BtnApply->setText("Apply");
  connect(BtnApply, &QPushButton::pressed, this, &SpkPageUiTest::SetStylesheet);

  BtnFetch = new QPushButton(this);
  BtnFetch->setObjectName("spk_pg_qsstest_btnfetch");
  BtnFetch->setText("Fetch Stylesheet");
  connect(BtnFetch, &QPushButton::pressed, this, &SpkPageUiTest::FetchStylesheet);

  HLayInputBtns = new QHBoxLayout;
  HLayInputBtns->setObjectName("spk_pg_qsstest_hlay_inputbtns");
  HLayInputBtns->addWidget(BtnFetch);
  HLayInputBtns->addWidget(BtnApply);

  Btn = new QPushButton(this);
  Btn->setObjectName("spk_pg_qsstest_button");
  Btn->setText("TestButton");

  Chk = new QCheckBox(this);
  Chk->setObjectName("spk_pg_qsstest_checkbox");
  Chk->setText("CheckBox");

  Rad = new QRadioButton(this);
  Rad->setObjectName("spk_pg_qsstest_radiobtn");
  Rad->setText("RadioButton");

  Loading = new SpkLoading(this);
  Loading->setObjectName("spk_pg_qsstest_loading");
  Loading->start();

  AppItem = new SpkAppItem(0, this);
  AppItem->setObjectName("spk_pg_qsstest_appitem");
  AppItem->SetTitle("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
  AppItem->SetDescription("Nam vehicula lacus vitae leo fermentum efficitur. "
                          "Phasellus finibus risus id aliquam pulvinar.");
  AppItem->SetIcon(QIcon::fromTheme("dialog-information").pixmap(72, 72));

  PopupText = new QLineEdit(this);
  PopupText->setObjectName("spk_pg_qsstest_poptext");
  PopupText->setText("Hello, world");

  ShowPopup = new QPushButton(this);
  ShowPopup->setText("Show Popup");
  connect(ShowPopup, &QPushButton::clicked, this, &SpkPageUiTest::ShowPopupSlot);

  ShowAbout = new QPushButton(this);
  ShowAbout->setText("Show About Dialog");
  connect(ShowAbout, &QPushButton::clicked, [](){ SpkAbout::Show(); });

  SlideV = new QSlider(this);
  SlideV->setObjectName("spk_pg_qsstest_slider_v");
  SlideV->setOrientation(Qt::Vertical);
  SlideV->setMaximum(1000);
  SlideV->setMinimum(0);

  SlideH = new QSlider(this);
  SlideH->setObjectName("spk_pg_qsstest_slider_h");
  SlideH->setOrientation(Qt::Horizontal);
  SlideH->setMaximum(1000);
  SlideH->setMinimum(0);

  VLayTestWidgets = new QVBoxLayout;
  VLayTestWidgets->setObjectName("spk_pg_qsstest_vlay_btn");
  VLayTestWidgets->addWidget(Btn);
  VLayTestWidgets->addWidget(Chk);
  VLayTestWidgets->addWidget(Rad);
  VLayTestWidgets->addWidget(Loading);
  VLayTestWidgets->addWidget(PopupText);
  VLayTestWidgets->addWidget(ShowPopup);
  VLayTestWidgets->addWidget(ShowAbout);
  VLayTestWidgets->addWidget(AppItem);

  Group = new QGroupBox(this);
  Group->setObjectName("spk_pg_qsstest_groupbox");
  Group->setTitle("GroupBox");
  Group->setLayout(VLayTestWidgets);

  VLayInput = new QVBoxLayout;
  VLayInput->setObjectName("spk_pg_qsstest_inputlay");
  VLayInput->addWidget(TextStylesheet);
  VLayInput->addLayout(HLayInputBtns);

  VLayWidgets = new QVBoxLayout;
  VLayWidgets->setObjectName("spk_pg_qsstest_widgetlay");
  VLayWidgets->addWidget(Group);
  VLayWidgets->addWidget(SlideH);

  HLay4Slider = new QHBoxLayout;
  HLay4Slider->setObjectName("spk_pg_qsstest_hlay_for_slider");
  HLay4Slider->addLayout(VLayWidgets);
  HLay4Slider->addWidget(SlideV);

  WidL = new QWidget(this);
  WidL->setObjectName("spk_pg_qsstest_widleft");
  WidL->setLayout(HLay4Slider);

  WidR = new QWidget(this);
  WidR->setObjectName("spk_pg_qsstest_widright");
  WidR->setLayout(VLayInput);

  addWidget(WidL);
  addWidget(WidR);
}

void SpkUi::SpkPageUiTest::SetStylesheet()
{
  qApp->setStyleSheet(TextStylesheet->toPlainText());
}

void SpkUi::SpkPageUiTest::FetchStylesheet()
{
  TextStylesheet->setPlainText(SpkUi::CurrentStylesheet);
}

void SpkUi::SpkPageUiTest::ShowPopupSlot()
{
  SpkUi::Popup->Show(PopupText->text());
}
