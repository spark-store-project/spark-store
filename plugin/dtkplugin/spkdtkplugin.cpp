#include <DGuiApplicationHelper>
#include <DPlatformWindowHandle>
#include <DPlatformTheme>
#include "spkdtkplugin_impl.h"

using Dtk::Widget::DPlatformWindowHandle;

void SpkDtkPluginImpl::Initialize()
{
  connect(DGuiApplicationHelper::instance()->systemTheme(),
          &DPlatformTheme::activeColorChanged,
          this,
          &SpkDtkPluginImpl::AccentColorChanged);
  connect(DGuiApplicationHelper::instance(),
          &DGuiApplicationHelper::themeTypeChanged,
          [&](Dtk::Gui::DGuiApplicationHelper::ColorType t)
          {
            this->DarkLightThemeChanged(t == Dtk::Gui::DGuiApplicationHelper::DarkType);
          });
}

void SpkDtkPluginImpl::addWindow(QWidget *w, QObject *parent)
{
  DPlatformWindowHandle *h = new DPlatformWindowHandle(w, parent);
  Q_UNUSED(h);
}

QColor SpkDtkPluginImpl::GetAccentColor()
{
  return DGuiApplicationHelper::instance()->systemTheme()->activeColor();
}

bool SpkDtkPluginImpl::GetIsDarkTheme()
{
  return DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
}
