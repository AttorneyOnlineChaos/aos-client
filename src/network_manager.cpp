#include "network_manager.h"

#include "core/logging.h"
#include "spritechat_defs.h"
#include "spritechat_info.h"

#include <QAbstractSocket>
#include <QDebug>
#include <QWebSocket>

spritechat::NetworkManager::NetworkManager(const theory::PacketFactory &packetFactory, QObject *parent)
    : QObject{parent}
    , _packetFactory{packetFactory}
{
  _socket.setFactory(&_packetFactory);

  connect(&_socket, &theory::CargoSocket::connectedToPeer, this, [this] { setStatus(Connected); });
  connect(&_socket, &theory::CargoSocket::disconnectedFromPeer, this, [this] { setStatus(NotConnected); });
  connect(&_socket, &theory::CargoSocket::errorOccurred, this, &NetworkManager::reportError);
  connect(&_socket, &theory::CargoSocket::pendingPacketAvailable, this, &NetworkManager::pendingPacketAvailable);
  connect(&_socket, &theory::CargoSocket::pong, this, &NetworkManager::pong);
}

spritechat::NetworkManager::Status spritechat::NetworkManager::status() const
{
  return _status;
}

std::optional<theory::CargoError> spritechat::NetworkManager::lastError() const
{
  return _socket.lastError();
}

void spritechat::NetworkManager::setStatus(Status status)
{
  if (_status == status)
  {
    return;
  }
  _status = status;
  Q_EMIT statusChanged(_status);
}

void spritechat::NetworkManager::connectToServer(const ServerBookmark &server)
{
  if (_status != NotConnected)
  {
    zWarning(log::network) << "close the connection before connecting to a new server";
    return;
  }

  zInfo(log::network) << QStringLiteral("Connecting to %1").arg(server.toString());

  _socket.setSocket(new QWebSocket);

  setStatus(Connecting);

  _socket.connectToUrl(server.join_url(), softwareUserAgent());
}

void spritechat::NetworkManager::disconnectFromServer()
{
  if (_status == NotConnected)
  {
    return;
  }

  if (_status == Connected)
  {
    _socket.close();
    return;
  }

  setStatus(NotConnected);
}

bool spritechat::NetworkManager::hasPendingPacket() const
{
  return _status == Connected && _socket.hasPendingPacket();
}

theory::PacketPointer spritechat::NetworkManager::nextPacket()
{
  if (_status != Connected)
  {
    return nullptr;
  }

  return _socket.nextPacket();
}

void spritechat::NetworkManager::shipPacket(const theory::Packet &packet)
{
  if (_status != Connected)
  {
    zCritical(log::network) << "Failed to ship packet; not connected.";
    return;
  }

  _socket.shipPacket(packet);
}

void spritechat::NetworkManager::ping()
{
  _socket.ping();
}

void spritechat::NetworkManager::reportError(const theory::CargoError &error)
{
  zWarning(log::network) << QStringLiteral("connection error: %1").arg(error.toString());

  Q_EMIT errorOccurred(error);
}
