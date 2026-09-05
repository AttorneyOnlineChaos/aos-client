#pragma once

#include "protocol/server_settings.h"

#include <QObject>

namespace spritechat
{
class ServerSettingsHandle : public QObject
{
  Q_OBJECT

public:
  explicit ServerSettingsHandle(QObject *parent = nullptr);

  void setSettings(const theory::ServerSettings &incoming);
  void clear();

  const theory::ServerSettings *operator->() const;

Q_SIGNALS:
  void settingsChanged();

private:
  theory::ServerSettings _settings;
};
} // namespace spritechat
