#pragma once

#include "aoapplication.h"
#include "core/pointer_types.h"
#include "datatypes.h"
#include "network/packet_transmitter.h"
#include "player_registry.h"
#include "widgets/moderator_dialog.h"

#include <QList>
#include <QListWidget>
#include <QMap>

namespace spritechat
{
class PlayerListWidget : public QListWidget
{
  Q_OBJECT
public:
  explicit PlayerListWidget(AOApplication *ao_app, PlayerRegistry &player_registry, theory::PacketTransmitter &transport, QWidget *parent = nullptr);

  void setArea(theory::AreaId area);
  void setAuthenticated(bool f_state);

  void reloadPlayers();

private:
  AOApplication *ao_app;
  PlayerRegistry &m_registry;
  theory::PacketTransmitter &m_transport;
  QMap<theory::PlayerId, QListWidgetItem *> m_item_map;
  theory::Unique<ModeratorDialog> m_dialog;
  theory::AreaId m_area = theory::NoAreaId;
  bool m_is_authenticated = false;

  QString formatLabel(const PlayerInfo &data);

  void filterPlayerList();

private Q_SLOTS:
  void addPlayer(theory::PlayerId id);
  void removePlayer(theory::PlayerId id);
  void refreshPlayer(theory::PlayerId id);
  void clearPlayers();

  void onCustomContextMenuRequested(const QPoint &pos);
};
} // namespace spritechat
