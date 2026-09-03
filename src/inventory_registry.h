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
class InventoryRegistry : public QObject
{
  Q_OBJECT

public:
  explicit InventoryRegistry(QObject *parent = nullptr);

  void add(theory::InventoryId id);
  void remove(theory::InventoryId id);
  void update(theory::InventoryId id, const InventoryInfo &inventory);
  void clear();

  std::optional<InventoryInfo> inventory(theory::InventoryId id) const;
  QList<InventoryInfo> inventories() const;

  using Condition = std::function<bool(const InventoryInfo &)>;
  QList<InventoryInfo> inventoriesIf(const Condition &condition) const;

Q_SIGNALS:
  void added(theory::InventoryId id);
  void removed(theory::InventoryId id);
  void updated(theory::InventoryId id);
  void cleared();

private:
  QMap<theory::InventoryId, InventoryInfo> _map;
};
} // namespace spritechat
