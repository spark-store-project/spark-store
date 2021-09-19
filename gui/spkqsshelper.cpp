
#include "spkqsshelper.h"

const std::list<SpkUi::Qss::ColorSetIndex> SpkUi::Qss::AccentColorExceptions
{
  AccentColor,
  AccentColorHighlighted,
  TextOnAccentColor,
};

const std::map<SpkUi::Qss::ColorSetIndex, const char *> SpkUi::Qss::ColorSet2Token
{
  { GlobalBgnd, "GBG_" },
  { ControlsBgnd, "CBG_" },
  { ControlsBgndHighlighted, "CBGH" },
  { SelectionBgnd, "ACC_" },
  { SelectionBgndHighlighted, "ACCH" },
  { LightCtrlsGradLight, "LCTL1" },
  { LightCtrlsGradDark, "LCTL2" },
  { LightCtrlsGradDarker, "LCTL3" },
  { LightCtrlsDisabledBackground, "LCTLD" },
  { DarkCtrlsGradLight, "DCTL1" },
  { DarkCtrlsGradDark, "DCTL2" },
  { DarkCtrlsGradDarker, "DCTL3" },
  { DarkCtrlsDisabledBackground, "DCTLD" },
  { TextOnSelection, "TXACC" },
  { TextOnGlobalBgnd, "TXGBG" },
  { TextOnControlsBgnd, "TXCBG" },
  { TextLighter, "TXL1" },
  { TextEvenLighter, "TXL2" },
  { TextDisabled, "TXD" },
  { GlossyEdge, "GLS" },
  { ShadesEdge, "SHD" }
};

const std::map<SpkUi::Qss::ColorSetIndex, QColor> SpkUi::Qss::DarkColorSet
{
  { GlobalBgnd, 0x282828 },
  { ControlsBgnd, 0x323232 },
  { ControlsBgndHighlighted, 0xff0000 },
  { SelectionBgnd, 0x0070ff },
  { SelectionBgndHighlighted, QColor(0x0070ff).lighter(120) },
  { LightCtrlsGradLight, 0x6b6b6b },
  { LightCtrlsGradDark, 0x656565 },
  { LightCtrlsGradDarker, 0x606060 },
  { LightCtrlsDisabledBackground, 0x808080 },
  { DarkCtrlsGradLight, 0x404040 },
  { DarkCtrlsGradDark, 0x383838 },
  { DarkCtrlsGradDarker, 0x323232 },
  { DarkCtrlsDisabledBackground, 0x525252 },
  { TextOnSelection, ColorTextOnBackground(0x0070ff) },
  { TextOnGlobalBgnd, ColorTextOnBackground(0x282828) },
  { TextOnControlsBgnd, ColorTextOnBackground(0x282828) },
  { TextLighter, 0xd5d5d5 },
  { TextEvenLighter, 0x505050 },
  { TextDisabled, 0xbebebe },
  { GlossyEdge, 0x656565 },
  { ShadesEdge, 0x7b7b7b }
};

const std::map<SpkUi::Qss::ColorSetIndex, QColor> SpkUi::Qss::LightColorSet
{
  { GlobalBgnd, 0xf8f8f8 },
  { ControlsBgnd, 0xf8f8f8 },
  { ControlsBgndHighlighted, 0xff0000 },
  { SelectionBgnd, 0x0070ff },
  { SelectionBgndHighlighted, QColor(0x0070ff).lighter(120) },
  { LightCtrlsGradLight, 0xfbfbfb },
  { LightCtrlsGradDark, 0xf2f2f2 },
  { LightCtrlsGradDarker, 0xebebeb },
  { LightCtrlsDisabledBackground, 0xe0e0e0 },
  { DarkCtrlsGradLight, 0xe4e4e4 },
  { DarkCtrlsGradDark, 0xcecece },
  { DarkCtrlsGradDarker, 0xb8b8b8 },
  { DarkCtrlsDisabledBackground, 0xababab },
  { TextOnSelection, ColorTextOnBackground(0x0070ff) },
  { TextOnGlobalBgnd, ColorTextOnBackground(0xf8f8f8) },
  { TextOnControlsBgnd, ColorTextOnBackground(0xf8f8f8) },
  { TextLighter, 0x2a2a2a },
  { TextEvenLighter, 0xa0a0a0 },
  { TextDisabled, 0x8a8a8a },
  { GlossyEdge, 0xc5c5c5 },
  { ShadesEdge, 0x9d9d9d }
};