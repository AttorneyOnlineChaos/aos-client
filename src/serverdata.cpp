#include <serverdata.h>

#include <QUrl>
#include <QVariant>

bool spritechat::ServerData::get_feature(const BASE_FEATURE_SET &f_feature) const
{
  return get_feature(QVariant::fromValue(f_feature).toString());
}

bool spritechat::ServerData::get_feature(const QString &f_feature) const
{
  return m_features.contains(f_feature, Qt::CaseInsensitive);
}

void spritechat::ServerData::set_features(const QStringList &f_feature_list)
{
  m_features = f_feature_list;
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

  m_asset_url = f_asset_url;
}
