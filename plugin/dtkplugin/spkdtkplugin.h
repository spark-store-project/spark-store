
#pragma once

#include <QWidget>

class SpkDtkPlugin : public QObject
{
    Q_OBJECT
  public:
    virtual ~SpkDtkPlugin() = default;
    virtual void Initialize() = 0;
    virtual void addWindow(QWidget* w, QObject* parent) = 0;
    virtual QColor GetAccentColor() = 0;
    virtual bool GetIsDarkTheme() = 0;

  signals:
    void AccentColorChanged(QColor);
    void DarkLightThemeChanged(bool isDark);
};
QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(SpkDtkPlugin, "org.spark-store.client.dtkplugin")
QT_END_NAMESPACE
