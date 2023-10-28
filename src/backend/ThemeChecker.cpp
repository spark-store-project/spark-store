#include "ThemeChecker.h"

#include <QDBusConnection>
#include <QDebug>

Q_GLOBAL_STATIC(ThemeChecker, m_instance)

constexpr char kFreedesktopPortalServiceName[] = "org.freedesktop.portal.Desktop";
constexpr char kFreedesktopPortalServicePath[] = "/org/freedesktop/portal/desktop";
constexpr char kFreedesktopPortalSettingsInterface[] = "org.freedesktop.portal.Settings";

ThemeChecker::ThemeChecker(QObject *parent)
    : QObject(parent)
    , m_paletteType(Dtk::Gui::DGuiApplicationHelper::instance()->paletteType())
{
    m_interface = new QDBusInterface(kFreedesktopPortalServiceName,
                                     kFreedesktopPortalServicePath,
                                     kFreedesktopPortalSettingsInterface,
                                     QDBusConnection::sessionBus(),
                                     this);

    initThemeType();
    initConnections();
}

ThemeChecker *ThemeChecker::instance()
{
    return m_instance;
}

bool ThemeChecker::useDarkTheme()
{
    if (m_paletteType == Dtk::Gui::DGuiApplicationHelper::ColorType::UnknownType) {
        return m_themeType == Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType;
    } else {
        return m_paletteType == Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType;
    }
}

void ThemeChecker::initThemeType()
{
    QVariantList args { "org.freedesktop.appearance", "color-scheme" };
    QDBusMessage msg = m_interface->callWithArgumentList(QDBus::Block, "Read", args);
    if (msg.type() != QDBusMessage::MessageType::ReplyMessage || msg.arguments().size() < 1) {
        qWarning().noquote() << "Init color-scheme from D-Bus failed:" << msg.errorName() << msg.errorMessage();
        m_themeType = Dtk::Gui::DGuiApplicationHelper::toColorType(Dtk::Gui::DGuiApplicationHelper::fetchPalette(Dtk::Gui::DGuiApplicationHelper::instance()->applicationTheme()));
        return;
    }

    quint32 colorScheme = qvariant_cast<QDBusVariant>(qvariant_cast<QDBusVariant>(msg.arguments().first()).variant()).variant().toUInt();
    switch (colorScheme) {
    case 1:
        m_themeType = Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType;
        break;
    case 2:
        m_themeType = Dtk::Gui::DGuiApplicationHelper::ColorType::LightType;
        break;
    default:
        qInfo().noquote() << QString("color-scheme: %1, fetching themeType according to QPalette::Window").arg(colorScheme);
        m_themeType = Dtk::Gui::DGuiApplicationHelper::toColorType(Dtk::Gui::DGuiApplicationHelper::fetchPalette(Dtk::Gui::DGuiApplicationHelper::instance()->applicationTheme()));
        break;
    }

    if (m_paletteType == Dtk::Gui::DGuiApplicationHelper::ColorType::UnknownType) {
        Dtk::Gui::DGuiApplicationHelper::instance()->setApplicationPalette(Dtk::Gui::DGuiApplicationHelper::standardPalette(m_themeType));
    }
}

void ThemeChecker::initConnections()
{
    QDBusConnection::sessionBus().connect(kFreedesktopPortalServiceName,
                                          kFreedesktopPortalServicePath,
                                          kFreedesktopPortalSettingsInterface,
                                          "SettingChanged",
                                          this,
                                          SLOT(slotSettingChanged(const QString &, const QString &, const QDBusVariant &)));

    connect(Dtk::Gui::DGuiApplicationHelper::instance()->applicationTheme(), &Dtk::Gui::DPlatformTheme::themeNameChanged, this, &ThemeChecker::slotThemeNameChanged);
    connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::paletteTypeChanged, this, &ThemeChecker::slotPaletteTypeChanged);
}

void ThemeChecker::slotSettingChanged(const QString &_namespace, const QString &key, const QDBusVariant &variant)
{
    if (_namespace != "org.freedesktop.appearance" || key != "color-scheme") {
        return;
    }

    Dtk::Gui::DGuiApplicationHelper::ColorType colorType = Dtk::Gui::DGuiApplicationHelper::ColorType::UnknownType;

    quint32 colorScheme = variant.variant().toUInt();
    switch (colorScheme) {
    case 1:
        colorType = Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType;
        break;
    case 2:
        colorType = Dtk::Gui::DGuiApplicationHelper::ColorType::LightType;
        break;
    default:
        qInfo().noquote() << QString("color-scheme: %1, fetching themeType according to QPalette::Window").arg(colorScheme);
        colorType = Dtk::Gui::DGuiApplicationHelper::toColorType(Dtk::Gui::DGuiApplicationHelper::fetchPalette(Dtk::Gui::DGuiApplicationHelper::instance()->applicationTheme()));
        break;
    }

    if (m_paletteType != Dtk::Gui::DGuiApplicationHelper::ColorType::UnknownType) {
        m_themeType = colorType;
        return;
    }

    if (m_themeType != colorType) {
        Dtk::Gui::DGuiApplicationHelper::instance()->setApplicationPalette(Dtk::Gui::DGuiApplicationHelper::standardPalette(colorType));
        emit themeChanged(colorType == Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType);
        m_themeType = colorType;
    }
}

void ThemeChecker::slotThemeNameChanged(const QByteArray &theme)
{
    Dtk::Gui::DGuiApplicationHelper::ColorType themeType = Dtk::Gui::DGuiApplicationHelper::ColorType::LightType;
    if (theme.endsWith("dark")) {
        themeType = Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType;
    }

    if (m_paletteType != Dtk::Gui::DGuiApplicationHelper::ColorType::UnknownType) {
        m_themeType = themeType;
        return;
    }

    if (m_themeType != themeType) {
        Dtk::Gui::DGuiApplicationHelper::instance()->setApplicationPalette(Dtk::Gui::DGuiApplicationHelper::standardPalette(themeType));
        emit themeChanged(themeType == Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType);
        m_themeType = themeType;
    }
}

void ThemeChecker::slotPaletteTypeChanged(Dtk::Gui::DGuiApplicationHelper::ColorType paletteType)
{
    m_paletteType = paletteType;

    if (m_paletteType != Dtk::Gui::DGuiApplicationHelper::ColorType::UnknownType) {
        Dtk::Gui::DGuiApplicationHelper::instance()->setApplicationPalette(Dtk::Gui::DGuiApplicationHelper::standardPalette(m_paletteType));
        emit themeChanged(m_paletteType == Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType);
        return;
    }

    Dtk::Gui::DGuiApplicationHelper::instance()->setApplicationPalette(Dtk::Gui::DGuiApplicationHelper::standardPalette(m_themeType));
    emit themeChanged(m_themeType == Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType);
}
