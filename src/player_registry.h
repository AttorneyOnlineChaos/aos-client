#pragma once

#include "datatypes.h"
#include "game/game_defs.h"

#include <QList>
#include <QMap>
#include <QObject>

#include <functional>
#include <optional>

namespace spritechat
{
class PlayerRegistry : public QObject
{
  Q_OBJECT

public:
  explicit PlayerRegistry(QObject *parent = nullptr);

  void add(theory::ClientId id);
  void remove(theory::ClientId id);
  void update(theory::ClientId id, const PlayerInfo &player);
  void clear();

  std::optional<PlayerInfo> player(theory::ClientId id) const;
  QList<PlayerInfo> players() const;

  using Condition = std::function<bool(const PlayerInfo &)>;
  QList<PlayerInfo> playersIf(const Condition &condition) const;

Q_SIGNALS:
  void added(theory::ClientId id);
  void removed(theory::ClientId id);
  void updated(theory::ClientId id);
  void cleared();

private:
  QMap<theory::ClientId, PlayerInfo> _map;
};
} // namespace spritechat
