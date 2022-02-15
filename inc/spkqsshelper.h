
#pragma once

#include <map>
#include <list>
#include <QColor>

namespace SpkUi
{
  QColor ColorTextOnBackground(QColor);

  namespace Qss
  {
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
      LightCtrlsDisabledBackground,
      DarkCtrlsGradLight,
      DarkCtrlsGradDark,
      DarkCtrlsGradDarker,
      DarkCtrlsDisabledBackground,
      TextOnSelection, TextOnAccentColor = TextOnSelection,
      TextOnGlobalBgnd,
      TextOnControlsBgnd,
      TextLighter,
      TextEvenLighter,
      TextDisabled,
      GlossyEdge,
      ShadesEdge,
      ScrollBarNorm,
      ScrollBarHover,
      DivideLine,
    };

    extern const std::list<ColorSetIndex> AccentColorExceptions;

    extern const std::map<ColorSetIndex, const char *> ColorSet2Token;

    extern const std::map<ColorSetIndex, QColor>
    DarkColorSet, LightColorSet;

    using ColorSet = std::map<Qss::ColorSetIndex, QColor>;
  }
}
