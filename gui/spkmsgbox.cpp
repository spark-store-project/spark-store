
#include <QScrollArea>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include "spkui_general.h"
#include "spkmsgbox.h"
#include "spkstore.h"

// Suppress unwanted Clazy check warnings
// clazy:excludeall=connect-3arg-lambda,lambda-in-connect

const QSize SpkMsgBox::IconSize; // I don't know why I need it, compiler wants that

SpkMsgBox::SpkMsgBox(QWidget *parent)
{
  Q_UNUSED(parent);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
}

int SpkMsgBox::StaticExec(QString msg, QString title, QMessageBox::Icon icon,
                          QMessageBox::StandardButtons buttons, QString extra, bool expanded)
{
  SpkMsgBox *b = new SpkMsgBox(SpkStore::Instance->GetRootWindow());
  QWidget *wMsgWidget = new QWidget;
  QHBoxLayout *wMsg = new QHBoxLayout(wMsgWidget);
  QPushButton *wExpandBtn;
  QScrollArea *wExtraArea;
  QLabel *wMsgText = new QLabel,
         *wExtraInfo,
         *wIcon;
  int InitialHeight;
  bool hasextra = extra.length() != 0;

  if(icon)
  {
    wIcon = new QLabel;
    QIcon icon_;
    switch(icon)
    {
      case QMessageBox::Critical:
        icon_ = QIcon::fromTheme("dialog-error");
        break;
      case QMessageBox::Warning:
        icon_ = QIcon::fromTheme("dialog-warning");
        break;
      case QMessageBox::Information:
        icon_ = QIcon::fromTheme("dialog-information");
        break;
      case QMessageBox::Question:
        icon_ = QIcon::fromTheme("dialog-question");
        break;
      case QMessageBox::NoIcon:
        break;
    }
    if(icon)
      wIcon->setPixmap(icon_.pixmap(IconSize));
    wMsg->addWidget(wIcon);
  }
  wMsgText->setText(msg);
  wMsgText->setAlignment(Qt::AlignLeft);
  wMsg->addWidget(wMsgText);
  wMsg->setSpacing(10);
  wMsgWidget->setLayout(wMsg);
  b->AddWidget(wMsgWidget);
  b->GetTitleBar()->SetTitle(title);
  b->GetTitleBar()->SetOperationButton(SpkTitleBar::OperationButton::Close);
  b->SetResizable(false);
  b->SetCentralMargin(Margin, Margin, Margin, Margin);
  b->setMaximumSize(SpkUi::PrimaryScreenSize * 0.6);

  if(hasextra)
  {
    wExpandBtn = new QPushButton;
    wExtraInfo = new QLabel;
    wExtraArea = new QScrollArea;
    wExtraInfo->setText(extra);
    wExtraArea->setWidget(wExtraInfo);
    wExtraArea->setVisible(false);
    wExtraArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    wExpandBtn->setText(tr("Details"));
    wExpandBtn->setMaximumWidth(100);
    wExpandBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    wExpandBtn->setCheckable(true);
    wExpandBtn->setObjectName("styChkBtn");
    connect(wExpandBtn, &QPushButton::clicked,
            [&](){  // FIXME: hint doesn't change when visibility changes, this is a quirky hack
                    wExtraArea->setVisible(wExpandBtn->isChecked());
                    if(wExpandBtn->isChecked())
                      b->setFixedHeight(b->sizeHint().height());
                    else
                      b->setFixedHeight(InitialHeight);
                 });
    b->mBtnLay->addWidget(wExpandBtn);
    b->mBtnLay->addStretch();
    b->AddWidget(wExtraArea);
  }

  b->AddSpacing(3);
  AddButtons(b, buttons);
  if(hasextra)
  {
    b->mBtnLay->addStretch(); // Keep conventional buttons centered
    if(expanded)
      emit wExpandBtn->clicked();
  }
  InitialHeight = b->minimumSizeHint().height();
  auto pos = (SpkUi::PrimaryScreenSize - b->sizeHint()) / 2;
  b->move(pos.width(), pos.height());
  b->setWindowModality(Qt::ApplicationModal);
  b->setFixedSize(b->sizeHint());

  auto r = b->Exec();
  if(r != -1)
     r = b->mButtonList[r]; // Retrieve the correct button

  delete b;
  return r;
}

void SpkMsgBox::AddButtons(SpkMsgBox *me, QMessageBox::StandardButtons b)
{
  // If anyone can do it better, please let me know, I wrote this on the airplane
  using btn = QMessageBox::StandardButton;
  if(!b) return;
  if(b.testFlag(btn::Ok)) { me->AddButton(tr("OK")); me->mButtonList << btn::Ok; };
  if(b.testFlag(btn::Cancel)) { me->AddButton(tr("Cancel")); me->mButtonList << btn::Cancel; };
  if(b.testFlag(btn::Yes)) { me->AddButton(tr("Yes")); me->mButtonList << btn::Yes; };
  if(b.testFlag(btn::No)) { me->AddButton(tr("No")); me->mButtonList << btn::No; };
  if(b.testFlag(btn::Apply)) { me->AddButton(tr("Apply")); me->mButtonList << btn::Apply; };
  if(b.testFlag(btn::Reset)) { me->AddButton(tr("Reset")); me->mButtonList << btn::Reset; };
  if(b.testFlag(btn::Abort)) { me->AddButton(tr("Abort")); me->mButtonList << btn::Abort; };
  if(b.testFlag(btn::Retry)) { me->AddButton(tr("Retry")); me->mButtonList << btn::Retry; };
  if(b.testFlag(btn::Ignore)) { me->AddButton(tr("Ignore")); me->mButtonList << btn::Ignore; };
  if(b.testFlag(btn::Reset)) { me->AddButton(tr("Reset")); me->mButtonList << btn::Reset; };
  if(b.testFlag(btn::Close)) { me->AddButton(tr("Close")); me->mButtonList << btn::Close; };
  if(b.testFlag(btn::Open)) { me->AddButton(tr("Open")); me->mButtonList << btn::Open; };
}
