#pragma once

#include "ao_track.h"
#include "game/game_defs.h"

#include <bass.h>

#include <QMutex>
#include <QUrl>

#include <optional>

namespace spritechat
{
class AOMusicPlayer
{
public:
  AOMusicPlayer() = default;
  ~AOMusicPlayer();

  void setMuted(bool enabled);
  void setVolume(int volume);

  void play(const AOTrack &track, theory::MusicEffects effects, bool repeat);
  void stop(theory::MusicEffects effects);

private:
  struct Stream
  {
    QWORD loopStart = 0;
    QWORD loopEnd = 0;
    bool repeat = false;
  };

  mutable QMutex _mutex;
  HSTREAM _handle = 0;
  int _volume = 0;
  bool _muted = false;

  static HSTREAM createStream(const QUrl &url);

  static void CALLBACK beginLoop(HSYNC handle, DWORD channel, DWORD data, void *user);
  static void CALLBACK finishLoop(HSYNC handle, DWORD channel, DWORD data, void *user);
  static void setupLoop(DWORD channel, const Stream &stream);

  static std::optional<QWORD> bytePosition(HSTREAM stream, qint64 frame);

  double calculateVolume() const;

  void destroyStream(bool fadeOut);
};
} // namespace spritechat
