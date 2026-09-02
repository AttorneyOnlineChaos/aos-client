#include "aomusicplayer.h"

#include "aoapplication.h"
#include "core/logging.h"
#include "options.h"
#include "spritechat_defs.h"

#include <bass.h>

#include <QByteArray>
#include <QMutexLocker>
#include <QString>

#include <optional>

spritechat::AOMusicPlayer::~AOMusicPlayer()
{
  QMutexLocker lock(&_mutex);
  destroyStream(false);
}

void spritechat::AOMusicPlayer::setMuted(bool enabled)
{
  QMutexLocker lock(&_mutex);
  _muted = enabled;
  BASS_ChannelSetAttribute(_handle, BASS_ATTRIB_VOL, calculateVolume());
}

void spritechat::AOMusicPlayer::setVolume(int volume)
{
  QMutexLocker lock(&_mutex);
  _volume = volume;
  BASS_ChannelSetAttribute(_handle, BASS_ATTRIB_VOL, calculateVolume());
}

void spritechat::AOMusicPlayer::play(const AOTrack &track, theory::MusicEffects effects, bool repeat)
{
  if (!track.url.isLocalFile() && !Options::getInstance().streamingEnabled())
  {
    zWarning(log::audio) << "streaming disabled:" << track.url.toString();
    stop(effects);
    return;
  }

  const HSTREAM handle = createStream(track.url);
  if (handle == 0)
  {
    zWarning(log::audio) << "failed to open" << track.url.toString() << "(bass error" << BASS_ErrorGetCode() << ")";
    stop(effects);
    return;
  }

  bool segments = false;
  QWORD introStart = 0;
  QWORD introEnd = 0;
  QWORD loopStart = 0;
  QWORD loopEnd = 0;
  if (track.sample)
  {
    const auto introStartPos = bytePosition(handle, track.sample->intro.start);
    const auto introEndPos = bytePosition(handle, track.sample->intro.end);
    if (introStartPos && introEndPos)
    {
      segments = true;
      introStart = introStartPos.value();
      introEnd = introEndPos.value();
    }
    else
    {
      zWarning(log::audio) << "sample" << track.sample->title << "intro exceeds track length, ignoring intro:" << track.url.toString();
    }

    const auto loopStartPos = bytePosition(handle, track.sample->loop.start);
    const auto loopEndPos = bytePosition(handle, track.sample->loop.end);
    if (loopStartPos && loopEndPos)
    {
      segments = true;
      loopStart = loopStartPos.value();
      loopEnd = loopEndPos.value();
    }
    else
    {
      zWarning(log::audio) << "sample" << track.sample->title << "loop exceeds track length, ignoring loop:" << track.url.toString();
    }
  }

  QMutexLocker lock(&_mutex);

  QWORD position = introStart;
  const bool synchronized = effects.testFlag(theory::SynchronizePosition) && BASS_ChannelIsActive(_handle) == BASS_ACTIVE_PLAYING;
  if (synchronized)
  {
    position = BASS_ChannelGetPosition(_handle, BASS_POS_BYTE);
  }

  destroyStream(effects.testFlag(theory::FadeOut));
  _handle = handle;

  if (position != 0)
  {
    BASS_ChannelSetPosition(handle, position, BASS_POS_BYTE);
  }

  if (!segments)
  {
    BASS_ChannelFlags(handle, repeat ? BASS_SAMPLE_LOOP : 0, BASS_SAMPLE_LOOP);
  }
  else if (synchronized)
  {
    setupLoop(handle, Stream{loopStart, loopEnd, repeat});
  }
  else
  {
    Stream *stream = new Stream{loopStart, loopEnd, repeat};
    if (introEnd != 0)
    {
      BASS_ChannelSetSync(handle, BASS_SYNC_POS | BASS_SYNC_MIXTIME | BASS_SYNC_ONETIME, introEnd, beginLoop, stream);
    }
    else
    {
      BASS_ChannelSetSync(handle, BASS_SYNC_END | BASS_SYNC_MIXTIME | BASS_SYNC_ONETIME, 0, beginLoop, stream);
    }
    BASS_ChannelSetSync(handle, BASS_SYNC_FREE, 0, finishLoop, stream);
  }

  BASS_ChannelSetSync(handle, BASS_SYNC_DEV_FAIL, 0, AOApplication::BASSreset, nullptr);

  if (effects.testFlag(theory::FadeIn))
  {
    constexpr int durationMs = 1000;
    BASS_ChannelSetAttribute(handle, BASS_ATTRIB_VOL, 0.0f);
    BASS_ChannelSlideAttribute(handle, BASS_ATTRIB_VOL, calculateVolume(), durationMs);
  }
  else
  {
    BASS_ChannelSetAttribute(handle, BASS_ATTRIB_VOL, calculateVolume());
  }
  BASS_ChannelPlay(handle, FALSE);
}

void spritechat::AOMusicPlayer::stop(theory::MusicEffects effects)
{
  QMutexLocker lock(&_mutex);
  destroyStream(effects.testFlag(theory::FadeOut));
}

void CALLBACK spritechat::AOMusicPlayer::beginLoop(HSYNC, DWORD channel, DWORD, void *user)
{
  const Stream *stream = static_cast<Stream *>(user);
  BASS_ChannelSetPosition(channel, stream->loopStart, BASS_POS_BYTE);
  setupLoop(channel, *stream);
}

void CALLBACK spritechat::AOMusicPlayer::finishLoop(HSYNC, DWORD, DWORD, void *user)
{
  delete static_cast<Stream *>(user);
}

void spritechat::AOMusicPlayer::setupLoop(DWORD channel, const Stream &stream)
{
  BASS_ChannelSetPosition(channel, stream.loopEnd, BASS_POS_END);
  BASS_ChannelSetPosition(channel, stream.loopStart, BASS_POS_LOOP);
  BASS_ChannelFlags(channel, stream.repeat ? BASS_SAMPLE_LOOP : 0, BASS_SAMPLE_LOOP);
}

HSTREAM spritechat::AOMusicPlayer::createStream(const QUrl &url)
{
  HSTREAM handle = 0;
  if (url.isLocalFile())
  {
    const QString fileName = url.toLocalFile();
    handle = BASS_StreamCreateFile(FALSE, fileName.utf16(), 0, 0, BASS_STREAM_AUTOFREE | BASS_STREAM_PRESCAN | BASS_ASYNCFILE | BASS_UNICODE);
  }
  else
  {
    const QByteArray encoded = url.toEncoded();
    handle = BASS_StreamCreateURL(encoded.constData(), 0, BASS_STREAM_AUTOFREE, nullptr, nullptr);
  }

  if (handle != 0 && Options::getInstance().audioOutputDevice() != "default")
  {
    BASS_ChannelSetDevice(handle, BASS_GetDevice());
  }
  return handle;
}

std::optional<QWORD> spritechat::AOMusicPlayer::bytePosition(HSTREAM stream, qint64 frame)
{
  BASS_CHANNELINFO info;
  BASS_ChannelGetInfo(stream, &info);

  QWORD frameSize = 2; // 16-bit
  if (info.flags & BASS_SAMPLE_8BITS)
  {
    frameSize = 1;
  }
  else if (info.flags & BASS_SAMPLE_FLOAT)
  {
    frameSize = 4;
  }
  frameSize *= info.chans;

  const QWORD length = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
  if (length == static_cast<QWORD>(-1))
  {
    return std::nullopt;
  }

  const qint64 frameCount = length / frameSize;
  if (frame >= frameCount)
  {
    return std::nullopt;
  }
  return frame * frameSize;
}

double spritechat::AOMusicPlayer::calculateVolume() const
{
  return _muted ? 0.0 : _volume / 100.0;
}

void spritechat::AOMusicPlayer::destroyStream(bool fadeOut)
{
  if (fadeOut && calculateVolume() > 0.0 && BASS_ChannelIsActive(_handle) == BASS_ACTIVE_PLAYING)
  {
    constexpr int durationMs = 4000;
    BASS_ChannelSlideAttribute(_handle, BASS_ATTRIB_VOL | BASS_SLIDE_LOG, -1.0f, durationMs);
  }
  else
  {
    BASS_ChannelStop(_handle);
  }
  _handle = 0;
}
