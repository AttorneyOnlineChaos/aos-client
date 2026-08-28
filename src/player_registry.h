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

  void add(theory::PlayerId id);
  void remove(theory::PlayerId id);
  void update(theory::PlayerId id, const PlayerInfo &player);
  void clear();

  std::optional<PlayerInfo> player(theory::PlayerId id) const;
  QList<PlayerInfo> players() const;

  using Condition = std::function<bool(const PlayerInfo &)>;
  QList<PlayerInfo> playersIf(const Condition &condition) const;

Q_SIGNALS:
  void added(theory::PlayerId id);
  void removed(theory::PlayerId id);
  void updated(theory::PlayerId id);
  void cleared();

private:
  QMap<theory::PlayerId, PlayerInfo> _map;
};
} // namespace spritechat
