#ifndef THEMECHECKER_H
#define THEMECHECKER_H

#include <QObject>
#include <QDBusVariant>
#include <QDBusInterface>

#include <DPlatformTheme>
#include <DGuiApplicationHelper>

class ThemeChecker : public QObject
{
    Q_OBJECT

public:
    explicit ThemeChecker(QObject *parent = nullptr);
    static ThemeChecker *instance();

    bool useDarkTheme();

private:
    void initThemeType();
    void initConnections();

signals:
    void themeChanged(bool isDark);

private slots:
    void slotSettingChanged(const QString &_namespace, const QString &key, const QDBusVariant &variant);
    void slotThemeNameChanged(const QByteArray &theme);
    void slotPaletteTypeChanged(Dtk::Gui::DGuiApplicationHelper::ColorType paletteType);

private:
    QDBusInterface *m_interface = nullptr;

    Dtk::Gui::DGuiApplicationHelper::ColorType m_paletteType;
    Dtk::Gui::DGuiApplicationHelper::ColorType m_themeType;
};

#endif // THEMECHECKER_H
