#include "server_bookmark.h"

#include "protocol/protocol_info.h"

QString spritechat::ServerBookmark::toString() const
{
  return QString("%1 (%2:%3)").arg((name.isEmpty() ? QStringLiteral("Unnamed Server") : name), address, QString::number(port));
}

QUrl spritechat::ServerBookmark::info_url() const
{
  QUrl url;
  url.setScheme(protocol == QStringLiteral("wss") ? QStringLiteral("https") : QStringLiteral("http"));
  url.setHost(address);
  url.setPort(port);
  url.setPath(QStringLiteral("/info"));
  return url;
}

QUrl spritechat::ServerBookmark::join_url() const
{
  QUrl url;
  url.setScheme(protocol == QStringLiteral("wss") ? QStringLiteral("wss") : QStringLiteral("ws"));
  url.setHost(address);
  url.setPort(port);
  url.setPath(QStringLiteral("/join"));
  return url;
}

bool spritechat::is_protocol_compatible(const theory::ServerInfo &info)
{
  return info.protocolName == theory::protocolName() && info.protocolVersion == theory::protocolVersion();
}
