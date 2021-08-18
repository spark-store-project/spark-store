#pragma once

#include <QWidget>
#include <QSplitter>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include "spkappitem.h"

#include "spkloading.h"

namespace SpkUi
{
  class SpkPageUiTest : public QSplitter
  {
      Q_OBJECT
    public:
      SpkPageUiTest(QWidget *parent = nullptr);
      QWidget *WidL, *WidR;
      QVBoxLayout *VLayWidgets,
                  *VLayTestWidgets,
                  *VLayInput;
      QHBoxLayout *HLay4Slider,
                  *HLayInputBtns;
      QTextEdit *TextStylesheet;
      QPushButton *BtnApply;
      QPushButton *BtnFetch;

      QPushButton *Btn;
      QCheckBox *Chk;
      QRadioButton *Rad;
      QGroupBox *Group;
      QSlider *SlideH;
      QSlider *SlideV;
      SpkLoading *Loading;
      SpkAppItem *AppItem;

      QLineEdit *PopupText;
      QPushButton *ShowPopup,
                  *ShowAbout;

    public slots:
      void SetStylesheet();
      void FetchStylesheet();
      void ShowPopupSlot();
  };
}
