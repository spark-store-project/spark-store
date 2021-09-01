
#include <QVBoxLayout>
#include <QApplication>
#include <QStyleHints>
#include <QPainterPath>
#include <QFile>
#include <QPushButton>
#include <QMouseEvent>
#include "spklogging.h"
#include "spkwindow.h"
#include "spkui_general.h"

#include <QDebug>

SpkWindow::SpkWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
  mUseCustomEvents = SpkUi::DtkPlugin == nullptr; // Put to the front, to prevent warnings
  if(SpkUi::DtkPlugin && !qgetenv("SPARK_NO_DXCB").toInt())
    SpkUi::DtkPlugin->addWindow(this, parent); // Register window to DXcb so we got Deepin
  else
    setWindowFlags(Qt::FramelessWindowHint); // Remove default title bar
  mCornerRadius = 5;
  mUserCentralWidget = nullptr;
  mResizable = true;
  mMoving = mResizing = false;
  mCloseHook = nullptr;
  mMaximized = windowState().testFlag(Qt::WindowMaximized);

  PopulateUi();
}

SpkWindow::~SpkWindow()
{

}

bool SpkWindow::event(QEvent *evt)
{
  // These custom events weren't useful for Deepin platform
  if(!mUseCustomEvents)
    return QMainWindow::event(evt);

  switch(evt->type())
  {
    case QEvent::WindowStateChange:
    {
      mMaximized = windowState().testFlag(Qt::WindowMaximized);
      if(mMaximized)
        mTitleBarComponent->mBtnMaxRestore->SetRole(SpkTitleBarDefaultButton::Restore);
      else
        mTitleBarComponent->mBtnMaxRestore->SetRole(SpkTitleBarDefaultButton::MaximizeRestore);
      break;
    }
    case QEvent::MouseButtonPress:
    {
      if(!mResizable) break;
      auto e = static_cast<QMouseEvent*>(evt);
      if(e->button() != Qt::LeftButton) break;
      auto edge = DetectEdgeOnThis(e->pos());
      if(edge)
      {
        mResizing = true;
        mEdgesBeingResized = edge;
        return true;
      }
      else
      {
        if(!QMainWindow::event(evt) || 1)
        {
          mMoveOffset = e->globalPos() - pos();
          mMoving = true;
          mResizing = false;
        }
        return true;
      }
      break;
    }
    case QEvent::MouseButtonRelease:
    {
      if(!mResizable) break;
      auto e = static_cast<QMouseEvent*>(evt);
      if(e->button() != Qt::LeftButton) break;
      mResizing = false;
      mMoving = false;
      unsetCursor();
      return true;
      break;
    }
    case QEvent::HoverMove:
    {
      if((mResizing || !mResizable) && !mMoving) break;
      if(mMaximized)
      {
        unsetCursor();
        break;
      }
      if(mResizable && !mMoving)
      {
        auto e = static_cast<QHoverEvent*>(evt);
        auto edge = DetectEdgeOnThis(e->pos());
        SetMouseCursor(edge);
      }
      break;
    }
    case QEvent::MouseMove:
    {
      if(mMaximized) break;
      auto e = static_cast<QMouseEvent*>(evt);
      if(mResizing && mResizable)
      {
        ResizeWindowByCursor(e->globalPos());
        return true; // Intercept resize movements
      }
      else if(mMoving)
      {
        move(e->globalPos() - mMoveOffset);
      }
      break;
    }
    default:
      ;
  }
  return QMainWindow::event(evt);
}

Qt::Edges SpkWindow::DetectEdgeOnThis(QPoint p)
{
  Qt::Edges edge;
  if(p.x() < BorderWidth) edge |= Qt::LeftEdge;
  if(p.x() > width() - BorderWidth) edge |= Qt::RightEdge;
  if(p.y() < BorderWidth) edge |= Qt::TopEdge;
  if(p.y() > height() - BorderWidth) edge |= Qt::BottomEdge;
  return edge;
}

void SpkWindow::SetMouseCursor(Qt::Edges e)
{
  switch(e)
  {
    case Qt::TopEdge:
    case Qt::BottomEdge:
      setCursor(Qt::SizeVerCursor);
      break;
    case Qt::LeftEdge:
    case Qt::RightEdge:
      setCursor(Qt::SizeHorCursor);
      break;
    case Qt::TopEdge | Qt::LeftEdge:
    case Qt::BottomEdge | Qt::RightEdge:
      setCursor(Qt::SizeFDiagCursor);
      break;
    case Qt::TopEdge | Qt::RightEdge:
    case Qt::BottomEdge | Qt::LeftEdge:
      setCursor(Qt::SizeBDiagCursor);
      break;
    default:
      unsetCursor();
  }
}

void SpkWindow::ResizeWindowByCursor(QPoint p)
{
  auto r_ = geometry(), r = r_;
  switch(mEdgesBeingResized)
  {
    case Qt::TopEdge | Qt::LeftEdge:
      r.setLeft(p.x());
      if(r.width() < minimumWidth()) // If smaller than minimum the window moves, so we stop it
        r.setLeft(r_.left());
    case Qt::TopEdge:
      r.setTop(p.y());
      if(r.height() < minimumHeight()) // Same
        r.setTop(r_.top());
      break;
    case Qt::BottomEdge | Qt::LeftEdge:
      r.setLeft(p.x());
      if(r.width() < minimumWidth())
        r.setLeft(r_.left());
    case Qt::BottomEdge:
      r.setBottom(p.y());
      if(r.height() < minimumHeight())
        r.setBottom(r_.bottom());
      break;
    case Qt::TopEdge | Qt::RightEdge:
      r.setTop(p.y());
      r.setRight(p.x());
      if(r.height() < minimumHeight())
        r.setTop(r_.top());
      if(r.width() < minimumWidth())
        r.setRight(r_.right());
      break;
    case Qt::BottomEdge | Qt::RightEdge:
      r.setBottom(p.y());
      if(r.height() < minimumHeight())
        r.setBottom(r_.bottom());
    case Qt::RightEdge:
      r.setRight(p.x());
      if(r.width() < minimumWidth())
        r.setRight(r_.right());
      break;
    case Qt::LeftEdge:
      r.setLeft(p.x());
      if(r.width() < minimumWidth())
        r.setLeft(r_.left());
      break;
    default:
      return;
  }

  setGeometry(r);
}

void SpkWindow::closeEvent(QCloseEvent *e)
{
  if(mCloseHook)
  {
    if(mCloseHook())
      e->accept();
    else
      e->ignore();
    emit Closed();
    return;
  }
  e->accept();
  emit Closed();
}

void SpkWindow::paintEvent(QPaintEvent *e)
{
  //FIXME: DOESN'T WORK!
//  QPainter painter(this);
//  painter.setBrush(QBrush(Qt::NoBrush));
//  painter.setPen(QPen(SpkUi::ColorLine));
//  QRect rect = this->rect();
//  rect.setWidth(rect.width() - 2);
//  rect.setHeight(rect.height() - 2);
//  rect.adjust(-1, -1, 1, 1);
//  painter.drawRect(rect);
//  QWidget::paintEvent(e);
}

void SpkWindow::SetCornerRadius(int radius)
{
  mCornerRadius = radius;
}

void SpkWindow::PopulateUi()
{
  mCentralWidget = new QWidget(this);
  mMainVLayout = new QVBoxLayout;
  mTitleBarComponent = new SpkTitleBar(this);

  mCentralWidget->setLayout(mMainVLayout);

  mMainVLayout->addWidget(mTitleBarComponent);
  mMainVLayout->setAlignment(Qt::AlignTop);
  mMainVLayout->setContentsMargins(0, 0, 0, 0);
  mMainVLayout->setSpacing(0);

  mTitleBarComponent->SetTitle(qAppName());
  mTitleBarComponent->SetUseIcon(false);
  mTitleBarComponent->SetLinkedWindow(this);

  setCentralWidget(mCentralWidget);
  setWindowFlag(Qt::NoDropShadowWindowHint, false);
}

void SpkWindow::SetCentralWidget(QWidget *widget)
{
  if(mUserCentralWidget)
    mMainVLayout->removeWidget(mUserCentralWidget);
  mUserCentralWidget = widget;
  mMainVLayout->addWidget(widget);
}

void SpkWindow::SetUseTitleBar(bool x)
{
  mTitleBarComponent->setVisible(x);
}

void SpkWindow::SetCentralMargin(int a, int b, int c, int d)
{
  if(mUserCentralWidget)
    mUserCentralWidget->setContentsMargins(a, b, c, d);
}

void SpkWindow::SetCloseHook(bool (*f)())
{
  mCloseHook = f;
}

void SpkWindow::ClearCloseHook()
{
  mCloseHook = nullptr;
}

void SpkWindow::RecalculateSize()
{
  mMainVLayout->activate();
}

bool SpkWindow::GetUseTitleBar()
{
  return mTitleBarComponent->isVisible();
}

SpkTitleBar *SpkWindow::GetTitleBar()
{
  return mTitleBarComponent;
}

SpkTitleBar *SpkWindow::SetTitleBar(SpkTitleBar *bar, bool replace)
{
  if(!bar)
    return nullptr;
  auto ret = mTitleBarComponent;
  mMainVLayout->removeWidget(mTitleBarComponent);
  if(replace)
    mMainVLayout->insertWidget(0, bar);
  mTitleBarComponent = bar;
  bar->SetLinkedWindow(this);
  return ret;
}
