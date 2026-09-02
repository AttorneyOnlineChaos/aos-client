#pragma once

#include "game/track_sheet.h"

#include <QUrl>

#include <optional>

namespace spritechat
{
struct AOTrack
{
  QUrl url;
  std::optional<theory::TrackSample> sample;
};
} // namespace spritechat
