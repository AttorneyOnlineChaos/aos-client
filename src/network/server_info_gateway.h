#pragma once

#include "protocol/server_info.h"
#include "server_bookmark.h"

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

  bool allowInsecureTls() const;
  void setAllowInsecureTls(bool allow);

Q_SIGNALS:
  void infoSettled();

private:
  static constexpr int REQUEST_COOLDOWN_MS = 10 * 1000;

  QNetworkAccessManager *_http;
  QPointer<QNetworkReply> _reply;
  QHash<QString, QPair<qint64, theory::ServerInfo>> _cache;

  ServerBookmark _server;
  theory::ServerInfo _info;
  bool _reachable = false;
  bool _compatible = false;
  bool _allowInsecureTls = false;

private Q_SLOTS:
  void processReply(QNetworkReply *reply);
};
} // namespace spritechat
