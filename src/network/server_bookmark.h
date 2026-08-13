#pragma once

#include "protocol/server_info.h"

#include <QString>
#include <QUrl>

namespace spritechat
{
class ServerBookmark
{
public:
  QString name;
  QString description;
  QString address;
  quint16 port = 0;
  QString protocol = "ws";

  QString toString() const;

  QUrl info_url() const;
  QUrl join_url() const;
};

bool is_protocol_compatible(const theory::ServerInfo &info);
} // namespace spritechat
