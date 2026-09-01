#include "animationlayer.h"

#include "aoapplication.h"
#include "core/json_codec.h"
#include "core/logging.h"
#include "options.h"
#include "spritechat_defs.h"

#include <QRectF>
#include <QThreadPool>

static QThreadPool *thread_pool;

spritechat::AnimationLayer::AnimationLayer(QWidget *parent)
    : QLabel(parent)
{
  setAlignment(Qt::AlignCenter);

  m_ticker = new QTimer(this);
  m_ticker->setSingleShot(true);
  m_ticker->setTimerType(Qt::PreciseTimer);

  connect(m_ticker, &QTimer::timeout, this, &AnimationLayer::frameTicker);

  if (!thread_pool)
  {
    thread_pool = new QThreadPool(qApp);
    thread_pool->setMaxThreadCount(8);
  }

  createLoader();
}

spritechat::AnimationLayer::~AnimationLayer()
{
  deleteLoader();
}

QString spritechat::AnimationLayer::fileName()
{
  return m_file_name;
}

void spritechat::AnimationLayer::setFileName(const QString &fileName)
{
  stopPlayback();
  m_file_name = fileName;
  if (m_file_name.trimmed().isEmpty())
  {
#ifdef DEBUG_MOVIE
    zWarning(log::viewport) << "called with empty string";
#endif
    m_file_name = QObject::tr("Invalid File");
  }
  resetData();
}

void spritechat::AnimationLayer::startPlayback()
{
  if (m_processing)
  {
#ifdef DEBUG_MOVIE
    zWarning(log::viewport) << "called while already processing";
#endif
    return;
  }
  resetData();
  m_processing = true;
  setVisible(true);
  Q_EMIT startedPlayback();
  frameTicker();
}

void spritechat::AnimationLayer::stopPlayback()
{
  if (m_ticker->isActive())
  {
    m_ticker->stop();
  }
  m_processing = false;
  if (m_reset_cache_when_stopped)
  {
    createLoader();
  }
  Q_EMIT stoppedPlayback();
}

void spritechat::AnimationLayer::restartPlayback()
{
  stopPlayback();
  startPlayback();
}

void spritechat::AnimationLayer::pausePlayback(bool enabled)
{
  if (m_pause == enabled)
  {
#ifdef DEBUG_MOVIE
    zWarning(log::viewport) << "called with identical state";
#endif
    return;
  }
  m_pause = enabled;
}

QSize spritechat::AnimationLayer::frameSize()
{
  return m_frame_size;
}

int spritechat::AnimationLayer::frameCount()
{
  return m_frame_count;
}

int spritechat::AnimationLayer::currentFrameNumber()
{
  return m_frame_number;
}

/**
 * @brief AnimationLayer::jumpToFrame
 * @param number The frame number to jump to. Must be in valid range. If the number is out of range, the method does nothing.
 * @details If frame number is valid and playback is processing, the frame will immediately be displayed.
 */
void spritechat::AnimationLayer::jumpToFrame(int number)
{
  if (number < 0 || number >= m_frame_count)
  {
#ifdef DEBUG_MOVIE
    zWarning(log::viewport) << "failed to jump to frame" << number << "(file:" << m_file_name << ", frame count:" << m_frame_count << ")";
#endif
    return;
  }

  bool is_processing = m_processing;
  if (m_ticker->isActive())
  {
    m_ticker->stop();
  }
  m_target_frame_number = number;
  if (is_processing)
  {
    frameTicker();
  }
}

bool spritechat::AnimationLayer::isPlayOnce()
{
  return m_play_once;
}

void spritechat::AnimationLayer::setPlayOnce(bool enabled)
{
  m_play_once = enabled;
}

void spritechat::AnimationLayer::setStretchToFit(bool enabled)
{
  m_stretch_to_fit = enabled;
}

void spritechat::AnimationLayer::setResetCacheWhenStopped(bool enabled)
{
  m_reset_cache_when_stopped = enabled;
}

void spritechat::AnimationLayer::setFlipped(bool enabled)
{
  m_flipped = enabled;
}

void spritechat::AnimationLayer::setResizeMode(RESIZE_MODE mode)
{
  m_resize_mode = mode;
}

void spritechat::AnimationLayer::setMinimumDurationPerFrame(int duration)
{
  m_minimum_duration = duration;
}

void spritechat::AnimationLayer::setMaximumDurationPerFrame(int duration)
{
  m_maximum_duration = duration;
}

void spritechat::AnimationLayer::setMaskingRect(QRect rect)
{
  m_mask_rect_hint = rect;
  calculateFrameGeometry();
}

void spritechat::AnimationLayer::resizeEvent(QResizeEvent *event)
{
  QLabel::resizeEvent(event);
  calculateFrameGeometry();
}

void spritechat::AnimationLayer::createLoader()
{
  deleteLoader();
  m_loader = new AnimationLoader(thread_pool);
}

void spritechat::AnimationLayer::deleteLoader()
{
  if (m_loader)
  {
    delete m_loader;
    m_loader = nullptr;
  }
}

void spritechat::AnimationLayer::resetData()
{
  m_first_frame = true;
  m_frame_number = 0;
  if (m_file_name != m_loader->loadedFileName())
  {
    m_loader->load(m_file_name);
  }
  m_frame_count = m_loader->frameCount();
  m_frame_size = m_loader->size();
  m_frame_rect = QRect(QPoint(0, 0), m_frame_size);
  m_ticker->stop();
  calculateFrameGeometry();
}

void spritechat::AnimationLayer::calculateFrameGeometry()
{
  m_mask_rect = QRect();
  m_scaled_frame_size = QSize();
  m_transformation_mode = Qt::SmoothTransformation;

  QSize widget_size = size();
  if (!widget_size.isValid() || !m_frame_size.isValid())
  {
    return;
  }

  if (m_stretch_to_fit)
  {
    m_scaled_frame_size = widget_size;
  }
  else
  {
    m_scaled_frame_size = m_frame_size;
    if (m_frame_rect.contains(m_mask_rect_hint))
    {
      m_mask_rect = m_mask_rect_hint;
      m_scaled_frame_size = m_mask_rect_hint.size();
    }

    double scale = double(widget_size.height()) / double(m_scaled_frame_size.height());
    m_scaled_frame_size *= scale;

    if (m_frame_size.height() < widget_size.height())
    {
      m_transformation_mode = Qt::FastTransformation;
    }
  }

  if (m_resize_mode == PIXEL_RESIZE_MODE)
  {
    m_transformation_mode = Qt::FastTransformation;
  }
  else if (m_resize_mode == SMOOTH_RESIZE_MODE)
  {
    m_transformation_mode = Qt::SmoothTransformation;
  }

  displayCurrentFrame();
}

void spritechat::AnimationLayer::finishPlayback()
{
  stopPlayback();
  Q_EMIT finishedPlayback();
}

void spritechat::AnimationLayer::prepareNextTick()
{
  int duration = qMax(m_minimum_duration, m_current_frame.duration);
  duration = (m_maximum_duration > 0) ? qMin(m_maximum_duration, duration) : duration;
  m_ticker->start(duration);
}

void spritechat::AnimationLayer::displayCurrentFrame()
{
  QPixmap image = m_current_frame.texture;

  if (m_frame_size.isValid())
  {
    if (m_mask_rect.isValid())
    {
      image = image.copy(m_mask_rect);
    }

    if (!image.isNull())
    {
      image = image.scaled(m_scaled_frame_size, Qt::IgnoreAspectRatio, m_transformation_mode);

      if (m_flipped)
      {
        image = image.transformed(QTransform().scale(-1.0, 1.0));
      }
    }
  }
  else
  {
    image = QPixmap(1, 1);
    image.fill(Qt::transparent);
  }

  setPixmap(image);
}

void spritechat::AnimationLayer::frameTicker()
{
  if (!m_processing)
  {
    return;
  }

  if (m_frame_count < 1)
  {
    if (m_play_once)
    {
      finishPlayback();
      return;
    }

    stopPlayback();
    return;
  }

  if (m_pause && !m_first_frame)
  {
    return;
  }

  if (m_frame_number == m_frame_count)
  {
    if (m_play_once)
    {
      finishPlayback();
      return;
    }

    if (m_frame_count > 1)
    {
      m_frame_number = 0;
    }
    else
    {
      return;
    }
  }

  m_first_frame = false;
  if (m_target_frame_number != -1)
  {
    m_frame_number = m_target_frame_number;
    m_target_frame_number = -1;
  }
  m_current_frame = m_loader->frame(m_frame_number);
  displayCurrentFrame();
  Q_EMIT frameNumberChanged(m_frame_number);
  ++m_frame_number;

  if (!m_pause)
  {
    prepareNextTick();
  }
}

spritechat::CharacterAnimationLayer::CharacterAnimationLayer(AOApplication *ao_app, QWidget *parent)
    : AnimationLayer(parent)
    , ao_app(ao_app)
{
  connect(this, &CharacterAnimationLayer::frameNumberChanged, this, &CharacterAnimationLayer::notifyFrameEffect);
  connect(this, &CharacterAnimationLayer::finishedPlayback, this, &CharacterAnimationLayer::notifyEmotePlaybackFinished);
}

void spritechat::CharacterAnimationLayer::loadCharacterEmote(const QString &character, const QString &fileName, EmoteType emoteType)
{
  auto is_dialog_emote = [](EmoteType emoteType) {
    return emoteType == IdleEmote || emoteType == TalkEmote;
  };

  bool synchronize_frame = false;
  const int previous_frame_count = frameCount();
  const int previous_frame_number = currentFrameNumber();
  if (m_character == character && m_emote == fileName && is_dialog_emote(m_emote_type) && is_dialog_emote(emoteType))
  {
    synchronize_frame = true;
  }

  m_character = character;
  m_emote = fileName;
  m_resolved_emote = fileName;
  m_emote_type = emoteType;

  QStringList prefixes;
  bool placeholder_fallback = false;
  bool play_once = false;
  switch (emoteType)
  {
  default:
    break;

  case PreEmote:
    play_once = true;
    break;

  case IdleEmote:
    prefixes << QStringLiteral("(a)") << QStringLiteral("(a)/");
    placeholder_fallback = true;
    break;

  case TalkEmote:
    prefixes << QStringLiteral("(b)") << QStringLiteral("(b)/");
    placeholder_fallback = true;
    break;

  case PostEmote:
    prefixes << QStringLiteral("(c)") << QStringLiteral("(c)/");
    break;
  }

  QList<VPath> path_list;
  QList<QString> prefixed_emote_list;
  for (const QString &prefix : std::as_const(prefixes))
  {
    path_list << ao_app->get_character_path(character, prefix + m_emote);
    prefixed_emote_list << prefix + m_emote;
  }
  path_list << ao_app->get_character_path(character, m_emote);
  prefixed_emote_list << m_emote;

  if (placeholder_fallback)
  {
    path_list << ao_app->get_character_path(character, QStringLiteral("placeholder"));
    prefixed_emote_list << QStringLiteral("placeholder");
    path_list << ao_app->get_theme_path("placeholder", ao_app->default_theme);
    prefixed_emote_list << QStringLiteral("placeholder");
  }

  int index = -1;
  QString file_path = ao_app->get_image_path(path_list, index);
  if (index != -1)
  {
    m_resolved_emote = prefixed_emote_list[index];
  }

  setFileName(file_path);
  setPlayOnce(play_once);
  setResizeMode(ao_app->get_scaling(ao_app->get_emote_property(character, fileName, "scaling")));
  setStretchToFit(ao_app->get_emote_property(character, fileName, "stretch").startsWith("true"));
  if (synchronize_frame && previous_frame_count == frameCount())
  {
    jumpToFrame(previous_frame_number);
  }
}

void spritechat::CharacterAnimationLayer::setFrameEffects(const QList<theory::EmoteCue> &cues)
{
  m_effects.clear();

  for (const theory::EmoteCue &cue : std::as_const(cues))
  {
    m_effects[cue.frame].append(cue);
  }
}

void spritechat::CharacterAnimationLayer::notifyEmotePlaybackFinished()
{
  if (m_emote_type == PreEmote || m_emote_type == PostEmote)
  {
    Q_EMIT finishedPreOrPostEmotePlayback();
  }
}

void spritechat::CharacterAnimationLayer::notifyFrameEffect(int frameNumber)
{
  auto it = m_effects.constFind(frameNumber);
  if (it != m_effects.constEnd())
  {
    for (const theory::EmoteCue &cue : std::as_const(*it))
    {
      if (cue.emote == m_resolved_emote)
      {
        switch (cue.type)
        {
        default:
          break;

        case theory::EmoteCue::Sound:
          {
            const auto sound = theory::decodeJson<theory::SoundEmoteCue>(cue.data);
            if (!sound.fileName.isEmpty())
            {
              Q_EMIT soundEffect(sound.fileName);
            }
            break;
          }

        case theory::EmoteCue::Shake:
          Q_EMIT shakeEffect();
          break;

        case theory::EmoteCue::Realization:
          Q_EMIT flashEffect();
          break;
        }
      }
    }
  }
}

spritechat::BackgroundAnimationLayer::BackgroundAnimationLayer(AOApplication *ao_app, QWidget *parent)
    : AnimationLayer(parent)
    , ao_app(ao_app)
{}

void spritechat::BackgroundAnimationLayer::loadAndPlayAnimation(const QString &fileName)
{
  QString file_path = ao_app->get_image_suffix(ao_app->get_background_path(fileName));
#ifdef DEBUG_MOVIE
  if (file_path.isEmpty())
  {
    zWarning(log::viewport) << "[BackgroundLayer] Failed to load background:" << fileName;
  }
  else if (file_path == this->fileName())
  {
    return;
  }
  else
  {
    zInfo(log::viewport) << "[BackgroundLayer] Loading background:" << file_path;
  }
#endif

  bool is_different_file = file_path != this->fileName();
  if (is_different_file)
  {
    setFileName(file_path);
  }

  VPath design_path = ao_app->get_background_path("design.ini");
  setResizeMode(ao_app->get_scaling(ao_app->read_design_ini("scaling", design_path)));
  setStretchToFit(ao_app->read_design_ini("stretch", design_path).startsWith("true"));

  if (is_different_file)
  {
    startPlayback();
  }
}

spritechat::SplashAnimationLayer::SplashAnimationLayer(AOApplication *ao_app, QWidget *parent)
    : AnimationLayer(parent)
    , ao_app(ao_app)
{
  connect(this, &SplashAnimationLayer::startedPlayback, this, &SplashAnimationLayer::show);
  connect(this, &SplashAnimationLayer::stoppedPlayback, this, &SplashAnimationLayer::hide);
}

void spritechat::SplashAnimationLayer::loadAndPlayAnimation(const QString &p_filename, const QString &p_charname, const QString &p_miscname)
{
  QString file_path = ao_app->get_image(p_filename, Options::getInstance().theme(), Options::getInstance().subTheme(), ao_app->default_theme, p_miscname, p_charname, "placeholder");
  setFileName(file_path);
  setResizeMode(ao_app->get_misc_scaling(p_miscname));
  startPlayback();
}

spritechat::EffectAnimationLayer::EffectAnimationLayer(AOApplication *ao_app, QWidget *parent)
    : AnimationLayer(parent)
    , ao_app(ao_app)
{
  connect(this, &EffectAnimationLayer::startedPlayback, this, &EffectAnimationLayer::show);
  connect(this, &EffectAnimationLayer::stoppedPlayback, this, &EffectAnimationLayer::maybeHide);
}

void spritechat::EffectAnimationLayer::loadAndPlayAnimation(const QString &p_filename, bool repeat)
{
  setFileName(p_filename);
  setPlayOnce(!repeat);
  startPlayback();
}

void spritechat::EffectAnimationLayer::setHideWhenStopped(bool enabled)
{
  m_hide_when_stopped = enabled;
}

void spritechat::EffectAnimationLayer::maybeHide()
{
  if (m_hide_when_stopped && isPlayOnce())
  {
    hide();
  }
}

spritechat::InterfaceAnimationLayer::InterfaceAnimationLayer(AOApplication *ao_app, QWidget *parent)
    : AnimationLayer(parent)
    , ao_app(ao_app)
{
  setStretchToFit(true);

  connect(this, &InterfaceAnimationLayer::startedPlayback, this, &InterfaceAnimationLayer::show);
  connect(this, &InterfaceAnimationLayer::stoppedPlayback, this, &InterfaceAnimationLayer::hide);
}

void spritechat::InterfaceAnimationLayer::loadAndPlayAnimation(const QString &fileName, const QString &miscName)
{
  QString file_path = ao_app->get_image(fileName, Options::getInstance().theme(), Options::getInstance().subTheme(), ao_app->default_theme, miscName);
  setFileName(file_path);
  startPlayback();
}

spritechat::StickerAnimationLayer::StickerAnimationLayer(AOApplication *ao_app, QWidget *parent)
    : AnimationLayer(parent)
    , ao_app(ao_app)
{
  connect(this, &StickerAnimationLayer::startedPlayback, this, &StickerAnimationLayer::show);
  connect(this, &StickerAnimationLayer::stoppedPlayback, this, &StickerAnimationLayer::hide);
}

void spritechat::StickerAnimationLayer::loadAndPlayAnimation(const QString &fileName)
{
  QString misc_file; // FIXME this is a bad name
  if (Options::getInstance().customChatboxEnabled())
  {
    misc_file = ao_app->get_chat(fileName);
  }

  QString file_path = ao_app->get_image("sticker/" + fileName, Options::getInstance().theme(), Options::getInstance().subTheme(), ao_app->default_theme, misc_file);
  setFileName(file_path);
  setResizeMode(ao_app->get_misc_scaling(misc_file));
  startPlayback();
}
