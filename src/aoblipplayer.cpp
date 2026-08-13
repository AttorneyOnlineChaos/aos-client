#include "aoblipplayer.h"

spritechat::AOBlipPlayer::AOBlipPlayer(AOApplication *ao_app)
    : ao_app(ao_app)
{}

void spritechat::AOBlipPlayer::setVolume(int value)
{
  m_volume = value;
  updateInternalVolume();
}

void spritechat::AOBlipPlayer::setMuted(bool enabled)
{
  m_muted = enabled;
  updateInternalVolume();
}

void spritechat::AOBlipPlayer::setBlip(const QString &blip)
{
  QString path = ao_app->get_sfx_suffix(ao_app->get_sounds_path(blip));
  for (int i = 0; i < STREAM_COUNT; ++i)
  {
    BASS_StreamFree(m_stream[i]);

    if (path.endsWith(".opus"))
    {
      m_stream[i] = BASS_OPUS_StreamCreateFile(FALSE, path.utf16(), 0, 0, BASS_UNICODE | BASS_ASYNCFILE);
    }
    else
    {
      m_stream[i] = BASS_StreamCreateFile(FALSE, path.utf16(), 0, 0, BASS_UNICODE | BASS_ASYNCFILE);
    }
  }

  updateInternalVolume();
}

void spritechat::AOBlipPlayer::playBlip()
{
  HSTREAM stream = m_stream[m_cycle];
  BASS_ChannelSetDevice(stream, BASS_GetDevice());
  BASS_ChannelPlay(stream, false);
  m_cycle = ++m_cycle % STREAM_COUNT;
}

void spritechat::AOBlipPlayer::updateInternalVolume()
{
  float volume = m_muted ? 0.0f : (m_volume * 0.01);
  for (int i = 0; i < STREAM_COUNT; ++i)
  {
    BASS_ChannelSetAttribute(m_stream[i], BASS_ATTRIB_VOL, volume);
  }
}
