#include "ao_track_library.h"

#include "core/logging.h"
#include "spritechat_defs.h"

#include <QFileInfo>
#include <QList>
#include <QStringList>
#include <QUrl>

spritechat::AOTrackLibrary::AOTrackLibrary(AssetLookup &assetLookup)
    : _assetLookup{assetLookup}
{}

void spritechat::AOTrackLibrary::reload()
{
  _tracks.clear();

  const QStringList fileNames = _assetLookup.get_real_paths(VPath("sounds/music/tracks.json"));
  for (const QString &fileName : fileNames)
  {
    QList<theory::TrackSheet> sheets;
    if (const auto error = theory::loadTrackSheets(fileName, sheets))
    {
      zWarning(log::asset) << error->toString();
      continue;
    }

    for (const theory::TrackSheet &sheet : sheets)
    {
      _tracks.insert(sheet.track.toLower(), sheet);
    }
  }
}

QList<theory::TrackSheet> spritechat::AOTrackLibrary::sheets() const
{
  return _tracks.values();
}

std::optional<theory::TrackSheet> spritechat::AOTrackLibrary::sheet(const QString &fileName) const
{
  const auto it = _tracks.constFind(fileName.toLower());
  if (it == _tracks.constEnd())
  {
    return std::nullopt;
  }
  return it.value();
}

spritechat::AOTrack spritechat::AOTrackLibrary::track(const QString &fileName, int sampleIndex) const
{
  AOTrack track;

  if (fileName.startsWith("http"))
  {
    track.url = QUrl(fileName);
    return track;
  }

  const QString realPath = _assetLookup.get_sfx_suffix(_assetLookup.get_music_path(fileName));
  if (realPath.isEmpty())
  {
    return track;
  }
  track.url = QUrl::fromLocalFile(realPath);

  auto it = _tracks.constFind(fileName.toLower());
  if (it == _tracks.constEnd())
  {
    it = _tracks.constFind(QStringLiteral("%1.%2").arg(fileName, QFileInfo(realPath).suffix()).toLower());
  }
  if (it != _tracks.constEnd())
  {
    track.sample = it->sample(sampleIndex);
  }
  return track;
}
