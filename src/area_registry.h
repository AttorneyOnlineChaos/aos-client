#pragma once

#include "datatypes.h"
#include "game/game_defs.h"

#include <QList>
#include <QMap>
#include <QObject>

#include <optional>

namespace spritechat
{
class AreaRegistry : public QObject
{
  Q_OBJECT

public:
  explicit AreaRegistry(QObject *parent = nullptr);

  void add(theory::AreaId id);
  void remove(theory::AreaId id);
  void update(theory::AreaId id, const AreaInfo &area);
  void clear();

  std::optional<AreaInfo> area(theory::AreaId id) const;
  QList<AreaInfo> areas() const;

Q_SIGNALS:
  void added(theory::AreaId id);
  void removed(theory::AreaId id);
  void updated(theory::AreaId id);
  void cleared();

private:
  QMap<theory::AreaId, AreaInfo> _map;
};
} // namespace spritechat
