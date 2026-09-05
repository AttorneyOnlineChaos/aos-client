#include "server_settings_handle.h"

spritechat::ServerSettingsHandle::ServerSettingsHandle(QObject *parent)
    : QObject{parent}
{}

void spritechat::ServerSettingsHandle::setSettings(const theory::ServerSettings &incoming)
{
  _settings = incoming;
  Q_EMIT settingsChanged();
}

void spritechat::ServerSettingsHandle::clear()
{
  _settings = theory::ServerSettings();
}

const theory::ServerSettings *spritechat::ServerSettingsHandle::operator->() const
{
  return &_settings;
}
