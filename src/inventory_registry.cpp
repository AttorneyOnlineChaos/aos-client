#include "inventory_registry.h"

spritechat::InventoryRegistry::InventoryRegistry(QObject *parent)
    : QObject{parent}
{}

void spritechat::InventoryRegistry::add(theory::InventoryId id)
{
  if (_map.contains(id))
  {
    return;
  }
  _map.insert(id, InventoryInfo{.id = id});
  Q_EMIT added(id);
}

void spritechat::InventoryRegistry::remove(theory::InventoryId id)
{
  const auto it = _map.find(id);
  if (it == _map.end())
  {
    return;
  }
  _map.erase(it);
  Q_EMIT removed(id);
}

void spritechat::InventoryRegistry::update(theory::InventoryId id, const InventoryInfo &inventory)
{
  const auto it = _map.find(id);
  if (it == _map.end())
  {
    return;
  }
  *it = inventory;
  Q_EMIT updated(id);
}

void spritechat::InventoryRegistry::clear()
{
  if (_map.isEmpty())
  {
    return;
  }
  _map.clear();
  Q_EMIT cleared();
}

std::optional<spritechat::InventoryInfo> spritechat::InventoryRegistry::inventory(theory::InventoryId id) const
{
  const auto it = _map.constFind(id);
  if (it == _map.constEnd())
  {
    return std::nullopt;
  }
  return *it;
}

QList<spritechat::InventoryInfo> spritechat::InventoryRegistry::inventories() const
{
  return _map.values();
}

QList<spritechat::InventoryInfo> spritechat::InventoryRegistry::inventoriesIf(const Condition &condition) const
{
  QList<InventoryInfo> result;
  for (const InventoryInfo &inventory : _map)
  {
    if (condition(inventory))
    {
      result.append(inventory);
    }
  }
  return result;
}
