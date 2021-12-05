//
// Created by rigoligo on 2021/5/8.
//

#pragma once

#include <QWidget>
#include <QString>
#include <QSize>
#include <QColor>
#include <map>

#include "spkqsshelper.h"
#include "dtk/spkdtkplugin.h"

namespace SpkUi
{

  enum SpkUiStyle { Invalid, Light, Dark };
  enum SpkButtonStyle { Normal = 0, Recommend, Warn };

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
      void SetDarkLightTheme(bool isDark);
  };

  extern UiMetaObject SpkUiMetaObject;
  extern SpkUiStyle CurrentStyle;
  extern QString StylesheetBase, CurrentStylesheet;
  extern QColor ColorLine, ColorBack,
                ColorBtnMaskUnselected, ///< SpkIconButton icon mask colors, unselected & selected
                ColorBtnMaskSelected;
  extern QSize PrimaryScreenSize;
  extern SpkDtkPlugin *DtkPlugin;
  extern QStyle *OldSystemStyle;
  extern std::map<Qss::ColorSetIndex, QColor> CurrentColorSet;

  extern SpkPopup *Popup;

  namespace States
  {
    extern bool IsDDE, IsUsingDtkPlugin;
  }

  namespace Priv
  {
    extern bool CrashHandlerActivated;
    using namespace SpkUi::Qss;
  }

  void Initialize();
  void GuessAppropriateTheme();
  void PrepareForDeepinDesktop();
  bool CheckIsDeepinDesktop();
  QString StylesheetFromColors(Qss::ColorSet);

  QIcon GetThemedIcon(QString);

  void CrashSignalHandler(int);

  void SetGlobalStyle(const SpkUiStyle, const bool aPreserveAccentColor);
};
