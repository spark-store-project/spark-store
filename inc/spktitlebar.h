//
// Created by rigoligo on 2021/5/8.
//

#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QPushButton>
#include <QMainWindow>

namespace SpkUi
{

}

class SpkWindow;

class SpkTitleBarDefaultButton : public QPushButton
{
    Q_OBJECT
  public:
    SpkTitleBarDefaultButton(QWidget* parent = nullptr);
    enum OperationButton { Minimize = 1, MaximizeRestore = 2, Close = 4, Restore = 3 };
    void SetRole(OperationButton);
    constexpr static int ButtonWidth = 60;

  protected:
    void paintEvent(QPaintEvent *e) override;

  private:
    OperationButton Role;
    void PaintSymbol(QPainter &);
};

class SpkTitleBar : public QWidget
{
    Q_OBJECT
    friend class SpkWindow;
  public:
    SpkTitleBar(QWidget *parent = nullptr);
    ~SpkTitleBar();
    using OperationButton = SpkTitleBarDefaultButton::OperationButton;
    void SetOperationButton(OperationButton);

    void SetIcon(QPixmap p) { mIcon->setPixmap(p); }
    void SetTitle(QString t) { mTitle->setText(t); }
    QString GetTitle() { return mTitle->text(); }
    void SetUseIcon(bool t) { mIcon->setVisible(t); }
    void SetLinkedWindow(SpkWindow *w) { mLinkedWindow = w; }

    QHBoxLayout *GetUserSpace() { return mUserSpace; }
  protected:
    bool event(QEvent*) override;

  private:
    QLabel *mIcon;
    QLabel *mTitle;
    SpkWindow *mLinkedWindow;
    QHBoxLayout *mMainLayout, *mUserSpace, *mBtnGroup;
    SpkTitleBarDefaultButton *mBtnMin, *mBtnMaxRestore, *mBtnClose;

  private slots:
    void CloseWindow();
    void MinimizeWindow();
    void MaximizeRestoreWindow();
};
