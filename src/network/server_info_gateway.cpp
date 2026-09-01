#include "server_info_gateway.h"

#include "core/json_codec.h"
#include "core/logging.h"
#include "spritechat_defs.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>

#include <functional>
#include <optional>

spritechat::ServerInfoGateway::ServerInfoGateway(QObject *parent)
    : QObject{parent}
{
  _http = new QNetworkAccessManager{this};
}

spritechat::ServerBookmark spritechat::ServerInfoGateway::server() const
{
  return _server;
}

theory::ServerInfo spritechat::ServerInfoGateway::info() const
{
  return _info;
}

bool spritechat::ServerInfoGateway::isReachable() const
{
  return _reachable;
}

bool spritechat::ServerInfoGateway::isCompatible() const
{
  return _compatible;
}

void spritechat::ServerInfoGateway::requestInfo(const ServerBookmark &server)
{
  _server = server;
  _info = theory::ServerInfo();
  _reachable = false;
  _compatible = false;

  const QString cache_key = QString("%1:%2").arg(server.address, QString::number(server.port));
  if (_cache.contains(cache_key))
  {
    const auto &entry = _cache[cache_key];
    if (QDateTime::currentMSecsSinceEpoch() - entry.first < RequestCooldown)
    {
      _info = entry.second;
      _reachable = true;
      _compatible = is_protocol_compatible(_info);
      Q_EMIT infoSettled();
      return;
    }
  }

  QNetworkReply *superseded = _reply;
  _reply = nullptr;
  if (superseded)
  {
    superseded->abort();
  }

  QNetworkRequest request(server.info_url());
  request.setTransferTimeout(RequestCooldown);

  QNetworkReply *reply = _http->get(request);
  _reply = reply;
  connect(reply, &QNetworkReply::finished, this, std::bind(&ServerInfoGateway::processReply, this, reply));
}

void spritechat::ServerInfoGateway::processReply(QNetworkReply *reply)
{
  reply->deleteLater();

  if (reply != _reply)
  {
    return;
  }
  _reply = nullptr;

  const int http_status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (reply->error() != QNetworkReply::NoError || http_status != 200 || reply->bytesAvailable() > ReplyLimit)
  {
    zDebug(log::network) << "Failed to get server info from" << reply->url() << "(" << reply->errorString() << ") (http status" << http_status << ")";
    Q_EMIT infoSettled();
    return;
  }

  const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
  if (!document.isObject())
  {
    zDebug(log::network) << "Invalid server info from" << reply->url();
    Q_EMIT infoSettled();
    return;
  }

  std::optional<theory::JsonCodecError> error;
  const auto info = theory::decodeJson<theory::ServerInfo>(document.object(), error);
  if (error)
  {
    zDebug(log::network) << "Invalid server info from" << reply->url() << ":" << error->toString();
    Q_EMIT infoSettled();
    return;
  }

  const QString cache_key = QString("%1:%2").arg(_server.address, QString::number(_server.port));
  _cache.insert(cache_key, {QDateTime::currentMSecsSinceEpoch(), info});

  _info = info;
  _reachable = true;
  _compatible = is_protocol_compatible(info);

  Q_EMIT infoSettled();
}
