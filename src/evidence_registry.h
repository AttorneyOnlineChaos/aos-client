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
class EvidenceRegistry : public QObject
{
  Q_OBJECT

public:
  explicit EvidenceRegistry(QObject *parent = nullptr);

  void add(theory::EvidenceId id);
  void remove(theory::EvidenceId id);
  void update(theory::EvidenceId id, const EvidenceInfo &item);
  void clear();

  std::optional<EvidenceInfo> evidence(theory::EvidenceId id) const;
  QList<EvidenceInfo> evidence() const;

  using Condition = std::function<bool(const EvidenceInfo &)>;
  QList<EvidenceInfo> evidenceIf(const Condition &condition) const;

Q_SIGNALS:
  void added(theory::EvidenceId id);
  void removed(theory::EvidenceId id);
  void updated(theory::EvidenceId id);
  void cleared();

private:
  QMap<theory::EvidenceId, EvidenceInfo> _map;
};
} // namespace spritechat
