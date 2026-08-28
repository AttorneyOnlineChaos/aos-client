#include "player_registry.h"

spritechat::PlayerRegistry::PlayerRegistry(QObject *parent)
    : QObject{parent}
{}

void spritechat::PlayerRegistry::add(theory::PlayerId id)
{
  if (_map.contains(id))
  {
    return;
  }
  _map.insert(id, PlayerInfo{.id = id});
  Q_EMIT added(id);
}

void spritechat::PlayerRegistry::remove(theory::PlayerId id)
{
  const auto it = _map.find(id);
  if (it == _map.end())
  {
    return;
  }
  _map.erase(it);
  Q_EMIT removed(id);
}

void spritechat::PlayerRegistry::update(theory::PlayerId id, const PlayerInfo &player)
{
  const auto it = _map.find(id);
  if (it == _map.end())
  {
    return;
  }
  *it = player;
  Q_EMIT updated(id);
}

void spritechat::PlayerRegistry::clear()
{
  if (_map.isEmpty())
  {
    return;
  }
  _map.clear();
  Q_EMIT cleared();
}

std::optional<spritechat::PlayerInfo> spritechat::PlayerRegistry::player(theory::PlayerId id) const
{
  const auto it = _map.constFind(id);
  if (it == _map.constEnd())
  {
    return std::nullopt;
  }
  return *it;
}

QList<spritechat::PlayerInfo> spritechat::PlayerRegistry::players() const
{
  return _map.values();
}

QList<spritechat::PlayerInfo> spritechat::PlayerRegistry::playersIf(const Condition &condition) const
{
  QList<PlayerInfo> result;
  for (const PlayerInfo &player : _map)
  {
    if (condition(player))
    {
      result.append(player);
    }
  }
  return result;
}
