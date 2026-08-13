#pragma once

#include "aomusictrack.h"

#include "game/game_defs.h"

#include <bass.h>

#include <QString>

namespace spritechat
{
class AOMusicPlayer
{
public:
  explicit AOMusicPlayer();
  virtual ~AOMusicPlayer();

  void setMuted(bool enabled);

  bool play(const AOMusicTrack &track, theory::MusicEffects effects, bool loopEnabled);
  void stop(theory::MusicEffects effects);

  void setVolume(int value);
  void setLoop(bool enabled);

  static void CALLBACK loopProc(HSYNC handle, DWORD channel, DWORD data, void *user);

private:
  struct Stream
  {
    int volume = 0;
    HSTREAM handle = 0;
    HSYNC loopSync = 0;
    quint64 byteLoopStart = 0;
    quint64 byteLoopEnd = 0;
  };

  Stream m_stream;
  bool m_muted = false;

  static void stopStream(const Stream &stream, bool fadeOut);
  static quint64 loopPosition(HSTREAM stream, const AOMusicTrack &track, double value);
};
} // namespace spritechat
