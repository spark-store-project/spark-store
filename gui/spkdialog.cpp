
#include "spkdialog.h"
#include <QEventLoop>

SpkDialog::SpkDialog(QWidget *parent) : SpkWindow(parent, Qt::Dialog)
{
  mDialogWidget = new QWidget;
  mMainVLay = new QVBoxLayout(mDialogWidget);
  mWidgetsVLay = new QVBoxLayout();
  mBtnLay = new QHBoxLayout();
  mBtnGroup = new QButtonGroup(this);

  mMainVLay->addLayout(mWidgetsVLay);
  mMainVLay->addLayout(mBtnLay);
  mMainVLay->setSizeConstraint(QLayout::SetMinimumSize);

  mBtnLay->setAlignment(Qt::AlignCenter);

  SetCentralWidget(mDialogWidget);

  // idClicked is not available on platforms we support, shouldn't change it
  connect(mBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
          this, &SpkDialog::ButtonPressed);
  connect(this, &SpkWindow::Closed, this, &SpkDialog::ForceClose);
}

SpkDialog::~SpkDialog()
{
  auto itp = mParentsList.begin();
  for(auto itw = mWidgetsList.begin(); itw != mWidgetsList.end(); itw++)
  {
    (*itw)->setParent(*(itp++));// We shall never take the ownership of these widgets
  }
  delete mDialogWidget;
}

void SpkDialog::AddButton(QString text, SpkUi::SpkButtonStyle style)
{
  auto b = new QPushButton();
  b->setText(text);
  b->setMinimumWidth(100);
  b->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  switch(style)
  {

    case SpkUi::SpkButtonStyle::Recommend:
      b->setObjectName("sty_recommendbtn");
      break;
    case SpkUi::SpkButtonStyle::Warn:
      b->setObjectName("sty_warnbtn");
      break;
    default:
      break;
  }
  mBtnLay->addWidget(b);
  mBtnGroup->addButton(b);
}

void SpkDialog::AddWidget(QWidget *w)
{
  // Adding a widget does not take the ownership.
  mWidgetsVLay->addWidget(w);
  mWidgetsList << w;
  mParentsList << w->parentWidget();
}

void SpkDialog::AddLayout(QLayout *w)
{
  mWidgetsVLay->addLayout(w);
  mWidgetsList << w;
  mParentsList << w->parentWidget();
}

void SpkDialog::AddSpacing(int a)
{
  mWidgetsVLay->addSpacing(a);
}

void SpkDialog::SetMargin(int a)
{
  mWidgetsVLay->setMargin(a);
}

int SpkDialog::Exec()
{
  QEventLoop loop;
  connect(this, &SpkDialog::ExitEventLoop, &loop, &QEventLoop::exit);
  connect(this, &SpkDialog::CloseWindow, this, &QMainWindow::close);
  show();
  return loop.exec();
}

void SpkDialog::ButtonPressed(int aBtnId)
{
  disconnect(this, &SpkWindow::Closed, this, &SpkDialog::ForceClose);
  emit ExitEventLoop(-aBtnId - 2);
  emit CloseWindow();
}

void SpkDialog::ForceClose()
{
  disconnect(this, &SpkDialog::CloseWindow, this, &QMainWindow::close);
  emit ExitEventLoop(-1);
}

void SpkDialog::SetMargin(int left, int top, int right, int bottom)
{
  mWidgetsVLay->setContentsMargins(left, top, right, bottom);
}
