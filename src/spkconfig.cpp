
#include "spkconfig.h"

SpkConfig::SpkConfig(QObject *parent, QString configPath) :
  QObject(parent),
  mSettings(configPath, QSettings::IniFormat, this)
{

}

SpkConfig::~SpkConfig()
{
  // TODO
}

bool SpkConfig::BindField(QString key, QString *value, QString defaultValue, std::function<bool(void)> callback)
{
  if(mStringBindMap.contains(key))
    return false;

  *value = mSettings.value(key, defaultValue).toString();
  mStringBindMap[key] = QPair<QString*, std::function<bool(void)>>(value, callback);
  return true;
}

bool SpkConfig::BindField(QString key, int *value, int defaultValue, std::function<bool(void)> callback)
{
  if(mIntBindMap.contains(key))
    return false;

  *value = mSettings.value(key, defaultValue).toInt(); // Read inital value
  mIntBindMap[key] = QPair<int*, std::function<bool(void)>>(value, callback);
  return true;
}

bool SpkConfig::SetField(QString key, QString value)
{
  auto it = mStringBindMap.find(key);
  if(it == mStringBindMap.end())
    return false;
  else
  {
    if(it->second) // Has a valid callback?
    {
      QString originalValue = *it->first; // Backup first in case of a failure
      *it->first = value; // Set the target
      if(!it->second()) // Failure, restore and give up
      {
        *it->first = originalValue;
        return false;
      }
      else // Success, set the mSettings and continue
      {
        mSettings.setValue(key, value);
        return true;
      }
    }

    // No valid callback, normal operations
    *it->first = value;
    mSettings.setValue(key, value);
    return true;
  }
}

bool SpkConfig::SetField(QString key, int value)
{
  auto it = mIntBindMap.find(key);
  if(it == mIntBindMap.end())
    return false;
  else
  {
    if(it->second) // Has a valid callback?
    {
      int originalValue = *it->first; // Backup first in case of a failure
      *it->first = value; // Set the target
      if(!it->second()) // Failure, restore and give up
      {
        *it->first = originalValue;
        return false;
      }
      else // Success, set the mSettings and continue
      {
        mSettings.setValue(key, value);
        return true;
      }
    }

    // No valid callback, normal operations
    *it->first = value;
    mSettings.setValue(key, value);
    return true;
  }
}

QVariant SpkConfig::ReadField(QString key, QVariant defaultValue)
{
  return mSettings.value(key, defaultValue);
}

void SpkConfig::SetSettings(QString key, QVariant value)
{
  mSettings.setValue(key, value);
}

void SpkConfig::Sync()
{
  mSettings.sync();
}
