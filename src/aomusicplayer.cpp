#include "aomusicplayer.h"

#include "aoapplication.h"
#include "options.h"

#include <bass.h>

#include <QUrl>

spritechat::AOMusicPlayer::AOMusicPlayer()
{}

spritechat::AOMusicPlayer::~AOMusicPlayer()
{
  BASS_ChannelStop(m_stream.handle);
}

bool spritechat::AOMusicPlayer::play(const AOMusicTrack &track, theory::MusicEffects effects, bool loopEnabled)
{
  Stream next;
  next.volume = m_stream.volume;

  quint32 flags = BASS_STREAM_AUTOFREE;
  if (loopEnabled)
  {
    flags |= BASS_SAMPLE_LOOP;
  }

  const bool is_stream = !track.url.isLocalFile();
  if (is_stream)
  {
    if (!Options::getInstance().streamingEnabled())
    {
      BASS_ChannelStop(m_stream.handle);
      return false;
    }
    next.handle = BASS_StreamCreateURL(track.url.toEncoded().toStdString().c_str(), 0, flags, nullptr, 0);
  }
  else
  {
    flags |= BASS_STREAM_PRESCAN | BASS_UNICODE | BASS_ASYNCFILE;

    QString f_path = track.url.toLocalFile();
    next.handle = BASS_StreamCreateFile(FALSE, f_path.utf16(), 0, 0, flags);
  }

  int error = BASS_ErrorGetCode();
  if (Options::getInstance().audioOutputDevice() != "default")
  {
    BASS_ChannelSetDevice(next.handle, BASS_GetDevice());
  }

  if (loopEnabled)
  {
    next.byteLoopStart = loopPosition(next.handle, track, track.loopStart);
    next.byteLoopEnd = loopPosition(next.handle, track, track.loopEnd);
  }

  if (BASS_ChannelIsActive(m_stream.handle) == BASS_ACTIVE_PLAYING && effects.testFlag(theory::SynchronizePosition))
  {
    DWORD oldstream = m_stream.handle;
    BASS_ChannelLock(oldstream, true);
    // Sync it with the new sample
    BASS_ChannelSetPosition(next.handle, BASS_ChannelGetPosition(oldstream, BASS_POS_BYTE), BASS_POS_BYTE);
    BASS_ChannelLock(oldstream, false);
  }

  stopStream(m_stream, effects.testFlag(theory::FadeOut));

  m_stream = next;
  BASS_ChannelPlay(m_stream.handle, false);
  if (effects.testFlag(theory::FadeIn))
  {
    // Fade in our sample
    BASS_ChannelSetAttribute(m_stream.handle, BASS_ATTRIB_VOL, 0);
    BASS_ChannelSlideAttribute(m_stream.handle, BASS_ATTRIB_VOL, m_stream.volume / 100.0f, 1000);
  }
  else
  {
    this->setVolume(m_stream.volume);
  }

  BASS_ChannelSetSync(m_stream.handle, BASS_SYNC_DEV_FAIL, 0, AOApplication::BASSreset, 0);

  this->setLoop(loopEnabled); // Have to do this here due to any
                              // crossfading-related changes, etc.

  // Cheap hack to see if file missing
  return error != BASS_ERROR_HANDLE;
}

quint64 spritechat::AOMusicPlayer::loopPosition(HSTREAM stream, const AOMusicTrack &track, double value)
{
  if (track.loopUnit == AOMusicTrack::LoopUnit::Second)
  {
    QWORD bytes = BASS_ChannelSeconds2Bytes(stream, value);
    if (bytes == -1)
    {
      return 0;
    }

    return bytes;
  }

  BASS_CHANNELINFO info;
  if (!BASS_ChannelGetInfo(stream, &info))
  {
    return 0;
  }

  int sample_size = 2;
  if (info.flags & BASS_SAMPLE_8BITS)
  {
    sample_size = 1;
  }
  else if (info.flags & BASS_SAMPLE_FLOAT)
  {
    sample_size = 4;
  }

  return value * sample_size * info.chans;
}

void spritechat::AOMusicPlayer::stopStream(const Stream &stream, bool fadeOut)
{
  if (fadeOut && stream.volume > 0 && BASS_ChannelIsActive(stream.handle) == BASS_ACTIVE_PLAYING)
  {
    // Fade out the other sample and stop it (due to -1)
    BASS_ChannelSlideAttribute(stream.handle, BASS_ATTRIB_VOL | BASS_SLIDE_LOG, -1, 4000);
  }
  else
  {
    BASS_ChannelStop(stream.handle); // Stop the sample since we don't need it anymore
  }
}

void spritechat::AOMusicPlayer::stop(theory::MusicEffects effects)
{
  stopStream(m_stream, effects.testFlag(theory::FadeOut));

  BASS_ChannelRemoveSync(m_stream.handle, m_stream.loopSync);
  m_stream.loopSync = 0;
  m_stream.byteLoopStart = 0;
  m_stream.byteLoopEnd = 0;
  m_stream.handle = 0;
}

void spritechat::AOMusicPlayer::setMuted(bool enabled)
{
  m_muted = enabled;
  // Update all volume based on the mute setting
  setVolume(m_stream.volume);
}

void spritechat::AOMusicPlayer::setVolume(int value)
{
  m_stream.volume = value;
  // If muted, volume will always be 0
  float volume = (m_stream.volume / 100.0f) * !m_muted;
  BASS_ChannelSetAttribute(m_stream.handle, BASS_ATTRIB_VOL, volume);
}

void CALLBACK spritechat::AOMusicPlayer::loopProc(HSYNC handle, DWORD channel, DWORD data, void *user)
{
  Q_UNUSED(handle);
  Q_UNUSED(data);
  QWORD loop_start = *(static_cast<quint64 *>(user));
  BASS_ChannelLock(channel, true);
  BASS_ChannelSetPosition(channel, loop_start, BASS_POS_BYTE);
  BASS_ChannelLock(channel, false);
}

void spritechat::AOMusicPlayer::setLoop(bool enabled)
{
  if (!enabled)
  {
    if (BASS_ChannelFlags(m_stream.handle, 0, 0) & BASS_SAMPLE_LOOP)
    {
      BASS_ChannelFlags(m_stream.handle, 0,
                        BASS_SAMPLE_LOOP); // remove the LOOP flag
    }
    BASS_ChannelRemoveSync(m_stream.handle, m_stream.loopSync);
    m_stream.loopSync = 0;
    return;
  }

  BASS_ChannelFlags(m_stream.handle, BASS_SAMPLE_LOOP,
                    BASS_SAMPLE_LOOP); // set the LOOP flag
  if (m_stream.loopSync != 0)
  {
    BASS_ChannelRemoveSync(m_stream.handle, m_stream.loopSync); // remove the sync
    m_stream.loopSync = 0;
  }

  if (m_stream.byteLoopStart < m_stream.byteLoopEnd)
  {
    // Loop when the endpoint is reached.
    m_stream.loopSync = BASS_ChannelSetSync(m_stream.handle, BASS_SYNC_POS | BASS_SYNC_MIXTIME, m_stream.byteLoopEnd, loopProc, &m_stream.byteLoopStart);
  }
  else
  {
    // Loop when the end of the file is reached.
    m_stream.loopSync = BASS_ChannelSetSync(m_stream.handle, BASS_SYNC_END | BASS_SYNC_MIXTIME, 0, loopProc, &m_stream.byteLoopStart);
  }
}
