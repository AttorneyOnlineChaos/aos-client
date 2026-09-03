#include "area_registry.h"

spritechat::AreaRegistry::AreaRegistry(QObject *parent)
    : QObject{parent}
{}

void spritechat::AreaRegistry::add(theory::AreaId id)
{
  if (_map.contains(id))
  {
    return;
  }
  _map.insert(id, AreaInfo{.id = id});
  Q_EMIT added(id);
}

void spritechat::AreaRegistry::remove(theory::AreaId id)
{
  if (_map.remove(id) > 0)
  {
    Q_EMIT removed(id);
  }
}

void spritechat::AreaRegistry::update(theory::AreaId id, const AreaInfo &area)
{
  if (!_map.contains(id))
  {
    return;
  }
  _map.insert(id, area);
  Q_EMIT updated(id);
}

void spritechat::AreaRegistry::clear()
{
  if (_map.isEmpty())
  {
    return;
  }
  _map.clear();
  Q_EMIT cleared();
}

std::optional<spritechat::AreaInfo> spritechat::AreaRegistry::area(theory::AreaId id) const
{
  auto it = _map.constFind(id);
  if (it == _map.constEnd())
  {
    return std::nullopt;
  }
  return *it;
}

QList<spritechat::AreaInfo> spritechat::AreaRegistry::areas() const
{
  return _map.values();
}

QList<spritechat::AreaInfo> spritechat::AreaRegistry::areasIf(const Condition &condition) const
{
  QList<AreaInfo> result;
  for (const AreaInfo &area : _map)
  {
    if (condition(area))
    {
      result.append(area);
    }
  }
  return result;
}
