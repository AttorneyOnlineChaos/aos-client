#pragma once

#include "ao_track.h"
#include "asset_lookup.h"
#include "game/track_sheet.h"

#include <QHash>
#include <QList>
#include <QString>

#include <optional>

namespace spritechat
{
class AOTrackLibrary
{
public:
  explicit AOTrackLibrary(AssetLookup &assetLookup);

  void reload();

  QList<theory::TrackSheet> sheets() const;
  std::optional<theory::TrackSheet> sheet(const QString &fileName) const;

  AOTrack track(const QString &fileName, int sampleIndex = 0) const;

private:
  AssetLookup &_assetLookup;
  QHash<QString, theory::TrackSheet> _tracks;
};
} // namespace spritechat
