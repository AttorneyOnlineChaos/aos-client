#pragma once

#include "server_bookmark.h"

#include "protocol/server_info.h"

#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>

namespace spritechat
{
class ServerInfoGateway : public QObject
{
  Q_OBJECT

public:
  explicit ServerInfoGateway(QObject *parent = nullptr);

  ServerBookmark server() const;
  theory::ServerInfo info() const;
  bool isReachable() const;
  bool isCompatible() const;

  void requestInfo(const ServerBookmark &server);

Q_SIGNALS:
  void infoSettled();

private:
  static constexpr int RequestCooldown = 10 * 1000;
  static constexpr int ReplyLimit = 64 * 1024;

  QNetworkAccessManager *_http;
  QPointer<QNetworkReply> _reply;
  QHash<QString, QPair<qint64, theory::ServerInfo>> _cache;

  ServerBookmark _server;
  theory::ServerInfo _info;
  bool _reachable = false;
  bool _compatible = false;

private Q_SLOTS:
  void processReply(QNetworkReply *reply);
};
} // namespace spritechat
