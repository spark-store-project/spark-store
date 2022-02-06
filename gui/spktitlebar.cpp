
#include <QEvent>
#include <QMouseEvent>
#include "spkwindow.h"
#include "spkui_general.h"
#include "spktitlebar.h"

SpkTitleBar::SpkTitleBar(QWidget *parent) : QWidget(parent)
{
  mLinkedWindow = nullptr;

  setMinimumHeight(48);
  setMaximumHeight(48);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  mIcon = new QLabel(this);
  mTitle = new QLabel(this);
  mIcon->setFixedSize(32, 32);

  mMainLayout = new QHBoxLayout(this);
  mUserSpace = new QHBoxLayout();
  mBtnGroup = new QHBoxLayout();
  mBtnMin = new SpkTitleBarDefaultButton(this);
  mBtnMaxRestore = new SpkTitleBarDefaultButton(this);
  mBtnClose= new SpkTitleBarDefaultButton(this);
  mMainLayout->setSpacing(8);
  mBtnGroup->setSpacing(0);

  mMainLayout->addSpacing(12);
  mMainLayout->addWidget(mIcon);
  mMainLayout->addWidget(mTitle);
  mMainLayout->addLayout(mUserSpace);
  mMainLayout->addLayout(mBtnGroup);
  mBtnGroup->addWidget(mBtnMin);
  mBtnGroup->addWidget(mBtnMaxRestore);
  mBtnGroup->addWidget(mBtnClose);

  mBtnMin->SetRole(OperationButton::Minimize);
  mBtnMaxRestore->SetRole(OperationButton::MaximizeRestore);
  mBtnClose->SetRole(OperationButton::Close);

  mMainLayout->setContentsMargins(0, 0, 0, 0);

  setLayout(mMainLayout);

  connect(mBtnClose, &QPushButton::clicked, this, &SpkTitleBar::CloseWindow);
  connect(mBtnMin, &QPushButton::clicked, this, &SpkTitleBar::MinimizeWindow);
  connect(mBtnMaxRestore, &QPushButton::clicked, this, &SpkTitleBar::MaximizeRestoreWindow);
}

SpkTitleBar::~SpkTitleBar()
{
  //qDebug() << "Freed title bar!";
}

void SpkTitleBar::SetOperationButton(OperationButton type)
{
  mBtnClose->setVisible(type & OperationButton::Close);
  mBtnMaxRestore->setVisible(type & OperationButton::MaximizeRestore);
  mBtnMin->setVisible(type & OperationButton::Minimize);
}

bool SpkTitleBar::event(QEvent *evt)
{
  switch(evt->type())
  {
    case QEvent::MouseButtonDblClick:
    {
      if(static_cast<QMouseEvent*>(evt)->button())
        emit mBtnMaxRestore->clicked();
      break;
    }
    default:;
  }
  return QWidget::event(evt);
}

void SpkTitleBar::CloseWindow()
{
  if(mLinkedWindow)
    mLinkedWindow->close();
}

void SpkTitleBar::MinimizeWindow()
{
  if(mLinkedWindow)
    mLinkedWindow->setWindowState(Qt::WindowMinimized);
}

void SpkTitleBar::MaximizeRestoreWindow()
{
  if(mLinkedWindow)
  {
    if(mLinkedWindow->windowState().testFlag(Qt::WindowMaximized))
      mLinkedWindow->setWindowState(mLinkedWindow->windowState() & ~Qt::WindowMaximized);
    else
      mLinkedWindow->setWindowState(Qt::WindowMaximized);
  }
}

SpkTitleBarDefaultButton::SpkTitleBarDefaultButton(QWidget* parent) : QPushButton(parent)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  setMaximumWidth(ButtonWidth);
  setMinimumWidth(ButtonWidth);
  setFocusPolicy(Qt::NoFocus);
}

void SpkTitleBarDefaultButton::paintEvent(QPaintEvent *e)
{
  QPushButton::paintEvent(e);
  QPainter painter;
  painter.begin(this);
  PaintSymbol(painter);
  painter.end();
}

void SpkTitleBarDefaultButton::PaintSymbol(QPainter &p)
{
  QPen pen(SpkUi::ColorLine);
  p.setPen(pen);
  auto mh = height() / 2, mw = width() / 2, h = height(), w = width();
  constexpr int fr = 10;
  switch(Role)
  {
    case Minimize:
      p.drawLine(mw - w / fr, mh, mw + w / fr, mh);
      break;
    case MaximizeRestore:
      p.drawRect(mw - w / fr, mh - h / fr,
                 w / fr * 2, h / fr * 2);
      break;
    case Restore:
      p.drawRect(mw - w / fr - 2, mh - w / fr + 2, w / fr * 2, h / fr * 2);
      p.drawLine(mw - w / fr, mh - h / fr - 2, mw + w / fr, mh - h / fr - 2);
      p.drawLine(mw + w / fr, mh - h / fr - 2, mw + w / fr, mh + h / fr - 2);
      break;
    case Close:
      p.drawLine(mw - h / fr * 1.3, mh - h / fr * 1.3,
                 mw + h / fr * 1.3, mh + h / fr * 1.3);
      p.drawLine(mw - h / fr * 1.3, mh + h / fr * 1.3,
                 mw + h / fr * 1.3, mh - h / fr * 1.3);
      break;
  }
}

void SpkTitleBarDefaultButton::SetRole(OperationButton role)
{
  Role = role;
}
