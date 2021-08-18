//
// Created by rigoligo on 2021/5/8.
//

#pragma once

#include <QWidget>
#include <QString>
#include <QSize>
#include <QColor>

#include "dtk/spkdtkplugin.h"

namespace SpkUi
{
  enum SpkUiStyle { Light, Dark };
  enum SpkButtonStyle { Normal = 0, Recommend, Warn };
  enum ColorSetIndex
  {
    GlobalBgnd = 0,
    ControlsBgnd,
    ControlsBgndHighlighted,
    SelectionBgnd, AccentColor = SelectionBgnd,
    SelectionBgndHighlighted, AccentColorHighlighted = SelectionBgndHighlighted,
    LightCtrlsGradLight,
    LightCtrlsGradDark,
    LightCtrlsGradDarker,
    DarkCtrlsGradLight,
    DarkCtrlsGradDark,
    TextOnSelection, TextOnAccentColor = TextOnSelection,
    TextOnGlobalBgnd,
    TextOnControlsBgnd,
    TextLighter
  };

  class SpkPopup;

  constexpr int StackTraceArraySize = 64;
  constexpr const char * const StoreIconName = "spark-store";

  class UiMetaObject : public QObject
  {
      Q_OBJECT
    private:
      static UiMetaObject *sGlobalInstance;
    public:
      UiMetaObject() {}
    public slots:
      void SetAccentColor(QColor);
  };

  extern UiMetaObject SpkUiMetaObject;
  extern SpkUiStyle CurrentStyle;
  extern QString StylesheetBase, CurrentStylesheet;
  extern QColor ColorLine, ColorBack;
  extern QSize PrimaryScreenSize;
  extern SpkDtkPlugin *DtkPlugin;
  extern QStyle *OldSystemStyle;
  extern QList<QColor> CurrentColorSet;

  extern SpkPopup *Popup;

  namespace States
  {
    extern bool IsDDE, IsUsingDtkPlugin;
  }

  namespace Priv
  {
    extern bool CrashHandlerActivated;
  }

  void Initialize();
  void GuessAppropriateTheme();
  void PrepareForDeepinDesktop();
  bool CheckIsDeepinDesktop();
  QString StylesheetFromColors(QList<QColor>);

  QIcon GetThemedIcon(QString);
  QColor ColorTextOnBackground(QColor);

  void CrashSignalHandler(int);

  void SetGlobalStyle(SpkUiStyle);
};
