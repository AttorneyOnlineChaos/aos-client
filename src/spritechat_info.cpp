#include "spritechat_info.h"

QString spritechat::softwareName()
{
  return QStringLiteral("spritechat");
}

QString spritechat::softwareDisplayName()
{
  return QStringLiteral("SpriteChat");
}

QVersionNumber spritechat::softwareVersion()
{
  return QVersionNumber{2, 11, 0};
}

QString spritechat::softwareUserAgent()
{
  return QStringLiteral("%1/%2").arg(softwareName(), softwareVersion().toString());
}
