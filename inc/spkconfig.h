#ifndef SPKCONFIG_H
#define SPKCONFIG_H

#include <QSettings>
#include <QHash>
#include <QPair>
#include <functional>

class SpkConfig : public QObject
{
    Q_OBJECT
  public:
    SpkConfig(QObject *parent, QString configPath);
    ~SpkConfig();

    /**
     * @brief BindField If a variable is bound to the specified key, then future chanegs via SetField
     *                  will modify the provided variable. A callback can also be specified to make
     *                  sure the chanegs are acceptable.
     * @param key
     * @param value A pointer to the variable to be bound
     * @param defaultValue
     * @param callback When SetField is called to modify this specific key associated with a callback,
     *                 the callback is called. If the callback returned false then the original value
     *                 is restored to the value target, and changes won't be saved in mSettings, and
     *                 SetField will return false too. It is used to ensure if the target can accept
     *                 the changes.
     * @return false when the key is already bound.
     */
    bool BindField(QString key, QString* value, QString defaultValue, std::function<bool(void)> callback = nullptr);
    bool BindField(QString key, int* value, int defaultValue, std::function<bool(void)> callback = nullptr);

    bool SetField(QString key, QString value);
    bool SetField(QString key, int value);

    // Wrapper of QSettings::value, used for "read once on startup" configurations
    QVariant ReadField(QString key, QVariant defaultValue);
    // Wrapper of QSettings::setValue, used for "set and restart to take effect" configurations
    void SetSettings(QString key, QVariant value);

  private:
    QSettings mSettings;

    QHash<QString, QPair<QString*, std::function<bool(void)>>> mStringBindMap;
    QHash<QString, QPair<int*,std::function<bool(void)>>> mIntBindMap;

};

#endif // SPKCONFIG_H
