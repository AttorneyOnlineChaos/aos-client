#pragma once

#include "network/server_bookmark.h"

#include "core/pointer_types.h"
#include "network/cargo_error.h"
#include "network/cargo_socket.h"
#include "network/packet_factory.h"
#include "network/packet_transmitter.h"

#include <QObject>

#include <optional>

namespace spritechat
{
class NetworkManager : public QObject, public theory::PacketTransmitter
{
  Q_OBJECT

public:
  enum Status
  {
    NotConnected,
    Connecting,
    Connected,
  };
  Q_ENUM(Status)

  explicit NetworkManager(const theory::PacketFactory &packetFactory, QObject *parent = nullptr);

  Status status() const;

  void connectToServer(const ServerBookmark &server);

  bool hasPendingPacket() const;
  theory::PacketPointer nextPacket();

  std::optional<theory::CargoError> lastError() const;

public Q_SLOTS:
  void disconnectFromServer();
  void shipPacket(const theory::Packet &packet) override;
  void ping();

Q_SIGNALS:
  void statusChanged(Status status);
  void errorOccurred(const theory::CargoError &error);
  void pendingPacketAvailable();
  void pong(quint64 elapsedTime);

private:
  const theory::PacketFactory &_packetFactory;

  theory::CargoSocket _socket;
  Status _status = NotConnected;

  void setStatus(Status status);

private Q_SLOTS:
  void reportError(const theory::CargoError &error);
};
} // namespace spritechat
