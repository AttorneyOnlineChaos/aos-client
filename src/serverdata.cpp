#include "serverdata.h"

#include <QUrl>

QString spritechat::ServerData::get_server_software() const
{
  return m_server_software;
}

void spritechat::ServerData::set_server_software(const QString &newServer_software)
{
  m_server_software = newServer_software;
}

QString spritechat::ServerData::get_asset_url() const
{
  return m_asset_url;
}

void spritechat::ServerData::set_asset_url(const QString &f_asset_url)
{
  QUrl l_asset_url = QUrl::fromPercentEncoding(f_asset_url.toUtf8());

  if (l_asset_url.isValid())
  {
    m_asset_url = l_asset_url.toString();
  }
}
