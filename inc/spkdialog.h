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
    void AddStretch(int a = 0);
    void SetMargin(int);
    void SetMargin(int, int, int, int);
    int  Exec();

  private slots:
    void ButtonPressed(int);
    void ForceClose();

  signals:
    void ExitEventLoop(int);
    void CloseWindow();

  protected:
    QWidget *mDialogWidget;
  private:
    QVBoxLayout *mMainVLay, *mWidgetsVLay;
    QHBoxLayout *mBtnLay;
    QButtonGroup *mBtnGroup;
    QList<QObject*> mWidgetsList, mParentsList;
};
