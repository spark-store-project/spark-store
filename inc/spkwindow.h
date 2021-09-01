//
// Created by rigoligo on 2021/5/8.
//

#pragma once

#include <QMainWindow>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCloseEvent>
#include "spktitlebar.h"

class SpkWindow : public QMainWindow
{
    Q_OBJECT
  public:
    enum SpkWindowStyle { Dark, Light };
    static constexpr int BorderWidth = 7;

  private:
    QWidget *mCentralWidget, *mUserCentralWidget;
    QVBoxLayout *mMainVLayout;
    SpkTitleBar *mTitleBarComponent;
    int mCornerRadius;
    bool mMoving, mResizing, mMaximized, mResizable;
    Qt::Edges mEdgesBeingResized;
    QPoint mMoveOffset;
    bool (*mCloseHook)(void);
    bool mUseCustomEvents;

  public:
    SpkWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~SpkWindow() override;
    void SetCentralWidget(QWidget *);
    bool GetUseTitleBar();
    bool GetResizable() { return mResizable; }
    void SetCloseHook(bool(*f)(void));

  public slots:
    void SetCornerRadius(int);
    void SetUseTitleBar(bool);
    void SetResizable(bool a) { mResizable = a; }
    void SetCentralMargin(int, int, int, int);
    void ClearCloseHook();
    void RecalculateSize();
    SpkTitleBar *GetTitleBar();
    SpkTitleBar *SetTitleBar(SpkTitleBar*, bool replace = false);

  signals:
    void Closed();

  protected:
    bool event(QEvent *) override;
    Qt::Edges DetectEdgeOnThis(QPoint);
    void SetMouseCursor(Qt::Edges);
    void ResizeWindowByCursor(QPoint);
    void closeEvent(QCloseEvent *) override;
    void paintEvent(QPaintEvent *) override;

  private:
    void PopulateUi();
};
