#include "evidence_registry.h"

spritechat::EvidenceRegistry::EvidenceRegistry(QObject *parent)
    : QObject{parent}
{}

void spritechat::EvidenceRegistry::add(theory::EvidenceId id)
{
  if (_map.contains(id))
  {
    return;
  }
  _map.insert(id, EvidenceInfo{.id = id});
  Q_EMIT added(id);
}

void spritechat::EvidenceRegistry::remove(theory::EvidenceId id)
{
  const auto it = _map.find(id);
  if (it == _map.end())
  {
    return;
  }
  _map.erase(it);
  Q_EMIT removed(id);
}

void spritechat::EvidenceRegistry::update(theory::EvidenceId id, const EvidenceInfo &item)
{
  const auto it = _map.find(id);
  if (it == _map.end())
  {
    return;
  }
  *it = item;
  Q_EMIT updated(id);
}

void spritechat::EvidenceRegistry::clear()
{
  if (_map.isEmpty())
  {
    return;
  }
  _map.clear();
  Q_EMIT cleared();
}

std::optional<spritechat::EvidenceInfo> spritechat::EvidenceRegistry::evidence(theory::EvidenceId id) const
{
  const auto it = _map.constFind(id);
  if (it == _map.constEnd())
  {
    return std::nullopt;
  }
  return *it;
}

QList<spritechat::EvidenceInfo> spritechat::EvidenceRegistry::evidence() const
{
  return _map.values();
}

QList<spritechat::EvidenceInfo> spritechat::EvidenceRegistry::evidenceIf(const Condition &condition) const
{
  QList<EvidenceInfo> result;
  for (const EvidenceInfo &item : _map)
  {
    if (condition(item))
    {
      result.append(item);
    }
  }
  return result;
}
