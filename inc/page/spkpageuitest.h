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
#include <QProgressBar>
#include "spkappitem.h"
#include "spkstretchlayout.h"
#include "page/spkpageappdetails.h"
#include "spkiconbutton.h"

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
      SpkIconButton *IconBtn;
      QProgressBar *Prog;
      SpkLoading *Loading;
      SpkAppItem *AppItem;
      SpkStretchLayout *DetailsLay;
      SpkDetailEntry *Detail1, *Detail2, *Detail3;
      QWidget *DetailsWidget;

      QLineEdit *PopupText;
      QPushButton *ShowPopup,
                  *ShowAbout,
                  *ShowPkgmgr;

    public slots:
      void SetStylesheet();
      void FetchStylesheet();
      void ShowPopupSlot();
  };
}
