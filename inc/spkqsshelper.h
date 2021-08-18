
#pragma once

#include <map>
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
      DarkCtrlsGradLight,
      DarkCtrlsGradDark,
      TextOnSelection, TextOnAccentColor = TextOnSelection,
      TextOnGlobalBgnd,
      TextOnControlsBgnd,
      TextLighter,
      GlossyEdge,
      ShadesEdge,
    };

    const std::map<ColorSetIndex, const char *> ColorSet2Token
    {
      { GlobalBgnd, "GBG_" },
      { ControlsBgnd, "CBG_" },
      { ControlsBgndHighlighted, "CBGH" },
      { SelectionBgnd, "ACC_" },
      { SelectionBgndHighlighted, "ACCH" },
      { LightCtrlsGradLight, "LCTL1" },
      { LightCtrlsGradDark, "LCTL2" },
      { LightCtrlsGradDarker, "LCTL3" },
      { DarkCtrlsGradLight, "DCTL1" },
      { DarkCtrlsGradDark, "DCTL2" },
      { TextOnSelection, "TXACC" },
      { TextOnGlobalBgnd, "TXGBG" },
      { TextOnControlsBgnd, "TXCBG" },
      { TextLighter, "TXL" },
      { GlossyEdge, "GLS" },
      { ShadesEdge, "SHD" }
    };

    const std::map<ColorSetIndex, QColor>
    DarkColorSet
    {
      { GlobalBgnd, 0x282828 },
      { ControlsBgnd, 0x323232 },
      { ControlsBgndHighlighted, 0xff0000 },
      { SelectionBgnd, 0x0070ff },
      { SelectionBgndHighlighted, QColor(0x0070ff).lighter(120) },
      { LightCtrlsGradLight, 0x6b6b6b },
      { LightCtrlsGradDark, 0x656565 },
      { LightCtrlsGradDarker, 0x606060 },
      { DarkCtrlsGradLight, 0x404040 },
      { DarkCtrlsGradDark, 0x383838 },
      { TextOnSelection, ColorTextOnBackground(0x0070ff) },
      { TextOnGlobalBgnd, ColorTextOnBackground(0x282828) },
      { TextOnControlsBgnd, ColorTextOnBackground(0x282828) },
      { TextLighter, 0xd5d5d5 },
      { GlossyEdge, 0x656565 },
      { ShadesEdge, 0x7b7b7b }
    },

    LightColorSet // TODO: Use real light color set
    {
    //          { GlobalBgnd, 0x282828 },
    //          { ControlsBgnd, 0x282828 },
    //          { ControlsBgndHighlighted, 0xff0000 },
    //          { SelectionBgnd, 0x0070ff },
    //          { SelectionBgndHighlighted, QColor(0x0070ff).lighter(120) },
    //          { LightCtrlsGradLight, 0x6b6b6b },
    //          { LightCtrlsGradDark, 0x656565 },
    //          { LightCtrlsGradDarker, 0x606060 },
    //          { DarkCtrlsGradLight, 0x404040 },
    //          { DarkCtrlsGradDark, 0x383838 },
    //          { TextOnSelection, ColorTextOnBackground(0x0070ff) },
    //          { TextOnGlobalBgnd, ColorTextOnBackground(0x282828) },
    //          { TextOnControlsBgnd, ColorTextOnBackground(0x282828) },
    //          { TextLighter, 0xd5d5d5 }
    };

    using ColorSet = std::map<Qss::ColorSetIndex, QColor>;
  }
}
