#pragma once

#include <QList>
#include <QButtonGroup>
#include "spkui_general.h"
#include "spktitlebar.h"
#include "spkwindow.h"

class SpkDialog : public SpkWindow
{
    Q_OBJECT
    friend class SpkMsgBox;
  public:
    SpkDialog(QWidget *parent = nullptr);
    ~SpkDialog();
    void AddButton(QString text, SpkUi::SpkButtonStyle style = SpkUi::SpkButtonStyle::Normal);
    void AddWidget(QWidget*);
    void AddLayout(QLayout*);
    void AddSpacing(int);
    void SetMargin(int);
    int  Exec();

  private slots:
    void ButtonPressed(int);
    void ForceClose();

  signals:
    void ExitEventLoop(int);
    void CloseWindow();

  private:
    QWidget *mDialogWidget;
    QVBoxLayout *mMainVLay, *mWidgetsVLay;
    QHBoxLayout *mBtnLay;
    QButtonGroup *mBtnGroup;
    QList<QObject*> mWidgetsList, mParentsList;
};
