#include "options.h"
#include "file_functions.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QLocale>
#include <QObject>
#include <QRegularExpression>
#include <QSize>

void spritechat::Options::migrateCallwords()
{
  // Bla bla, evil boilerplate.
  QStringList l_callwords;

  QFile l_file;
  l_file.setFileName(get_base_path() + "callwords.ini");

  if (!l_file.open(QIODevice::ReadOnly))
  {
    qWarning() << "Unable to migrate callwords : File not open.";
    return;
  }

  QTextStream in(&l_file);

  while (!in.atEnd())
  {
    QString line = in.readLine();
    l_callwords.append(line);
  }
  l_file.close();
  l_file.remove();

  setCallwords(l_callwords);
}

spritechat::Options::Options()
    : config(get_base_path() + "config.ini", QSettings::IniFormat, nullptr)
    , favorite(get_base_path() + "favorite_servers.ini", QSettings::IniFormat, nullptr)
{
  migrate();
}

/*! Migrate old configuration keys/values to a relevant format. */
void spritechat::Options::migrate()
{
  if (QFile::exists(get_base_path() + "callwords.ini"))
  {
    migrateCallwords();
  }
  if (config.contains("ooc_name"))
  {
    if (username().isEmpty())
    {
      config.setValue("default_username", config.value("ooc_name"));
    }
    config.remove("ooc_name");
  }

  if (config.contains("casing_enabled"))
  {
    config.remove("casing_enabled");
    config.remove("casing_defence_enabled");
    config.remove("casing_prosecution_enabled");
    config.remove("casing_judge_enabled");
    config.remove("casing_juror_enabled");
    config.remove("casing_steno_enabled");
    config.remove("casing_cm_enabled");
    config.remove("casing_can_host_cases");
  }
}

QString spritechat::Options::theme() const
{
  return config.value("theme", "AceAttorney2x").toString();
}

void spritechat::Options::setTheme(QString value)
{
  config.setValue("theme", value);
}

double spritechat::Options::themeScalingFactor() const
{
  double value = config.value("theme_scaling_factor", "1").toDouble();
  if (value < 0.1)
  {
    value = 0.1;
  }
  return value;
}

void spritechat::Options::setThemeScalingFactor(double value)
{
  config.setValue("theme_scaling_factor", value);
}

int spritechat::Options::blipRate() const
{
  return config.value("blip_rate", 2).toInt();
}

void spritechat::Options::setBlipRate(int value)
{
  config.setValue("blip_rate", value);
}

int spritechat::Options::musicVolume() const
{
  return config.value("default_music", 50).toInt();
}

void spritechat::Options::setMusicVolume(int value)
{
  config.setValue("default_music", value);
}

int spritechat::Options::sfxVolume() const
{
  return config.value("default_sfx", 50).toInt();
}

void spritechat::Options::setSfxVolume(int value)
{
  config.setValue("default_sfx", value);
}

int spritechat::Options::blipVolume() const
{
  return config.value("default_blip", 50).toInt();
}

void spritechat::Options::setBlipVolume(int value)
{
  config.setValue("default_blip", value);
}

int spritechat::Options::defaultSuppressAudio() const
{
  return config.value("suppress_audio", 50).toInt();
}

void spritechat::Options::setDefaultSupressedAudio(int value)
{
  config.setValue("suppress_audio", value);
}

int spritechat::Options::maxLogSize() const
{
  return config.value("log_maximum", 200).toInt();
}

void spritechat::Options::setMaxLogSize(int value)
{
  config.setValue("log_maximum", value);
}

int spritechat::Options::textStayTime() const
{
  return config.value("stay_time", 200).toInt();
}

void spritechat::Options::setTextStayTime(int value)
{
  config.setValue("stay_time", value);
}

int spritechat::Options::textCrawlSpeed() const
{
  return config.value("text_crawl", 40).toInt();
}

void spritechat::Options::setTextCrawlSpeed(int value)
{
  config.setValue("text_crawl", value);
}

int spritechat::Options::chatRateLimit() const
{
  return config.value("chat_ratelimit", 300).toInt();
}

void spritechat::Options::setChatRateLimit(int value)
{
  config.setValue("chat_ratelimit", value);
}

bool spritechat::Options::logDirectionDownwards() const
{
  return config.value("log_goes_downwards", true).toBool();
}

void spritechat::Options::setLogDirectionDownwards(bool value)
{
  config.setValue("log_goes_downwards", value);
}

bool spritechat::Options::logNewline() const
{
  return config.value("log_newline", false).toBool();
}

void spritechat::Options::setLogNewline(bool value)
{
  config.setValue("log_newline", value);
}

int spritechat::Options::logMargin() const
{
  return config.value("log_margin", 0).toInt();
}

void spritechat::Options::setLogMargin(int value)
{
  config.setValue("log_margin", value);
}

bool spritechat::Options::logTimestampEnabled() const
{
  return config.value("log_timestamp", false).toBool();
}

void spritechat::Options::setLogTimestampEnabled(bool value)
{
  config.setValue("log_timestamp", value);
}

QString spritechat::Options::logTimestampFormat() const
{
  return config.value("log_timestamp_format", "h:mm:ss AP").toString();
}

void spritechat::Options::setLogTimestampFormat(QString value)
{
  config.setValue("log_timestamp_format", value);
}

bool spritechat::Options::logIcActions() const
{
  return config.value("log_ic_actions", true).toBool();
}

void spritechat::Options::setLogIcActions(bool value)
{
  config.setValue("log_ic_actions", value);
}

bool spritechat::Options::customShownameEnabled() const
{
  return config.value("show_custom_shownames", true).toBool();
}

void spritechat::Options::setCustomShownameEnabled(bool value)
{
  config.setValue("show_custom_shownames", value);
}

QString spritechat::Options::username() const
{
  return config.value("default_username", "").value<QString>();
}

void spritechat::Options::setUsername(QString value)
{
  config.setValue("default_username", value);
}

QString spritechat::Options::shownameOnJoin() const
{
  return config.value("default_showname", "").toString();
}

void spritechat::Options::setShownameOnJoin(QString value)
{
  config.setValue("default_showname", value);
}

QString spritechat::Options::audioOutputDevice() const
{
  return config.value("default_audio_device", "default").toString();
}

void spritechat::Options::setAudioOutputDevice(QString value)
{
  config.setValue("default_audio_device", value);
}

bool spritechat::Options::blankBlip() const
{
  return config.value("blank_blip", false).toBool();
}

void spritechat::Options::setBlankBlip(bool value)
{
  config.setValue("blank_blip", value);
}

bool spritechat::Options::loopingSfx() const
{
  return config.value("looping_sfx", true).toBool();
}

void spritechat::Options::setLoopingSfx(bool value)
{
  config.setValue("looping_sfx", value);
}

bool spritechat::Options::objectionStopMusic() const
{
  return config.value("objection_stop_music", false).toBool();
}

void spritechat::Options::setObjectionStopMusic(bool value)
{
  config.setValue("objection_stop_music", value);
}

bool spritechat::Options::streamingEnabled() const
{
  return config.value("streaming_enabled", true).toBool();
}

void spritechat::Options::setStreamingEnabled(bool value)
{
  config.setValue("streaming_enabled", value);
}

bool spritechat::Options::objectionSkipQueueEnabled() const
{
  return config.value("instant_objection", true).toBool();
}

void spritechat::Options::setObjectionSkipQueueEnabled(bool value)
{
  config.setValue("instant_objection", value);
}

bool spritechat::Options::desynchronisedLogsEnabled() const
{
  return config.value("desync_logs", false).toBool();
}

void spritechat::Options::setDesynchronisedLogsEnabled(bool value)
{
  config.setValue("desync_logs", value);
}

bool spritechat::Options::shakeEnabled() const
{
  return config.value("shake", true).toBool();
}

void spritechat::Options::setShakeEnabled(bool value)
{
  config.setValue("shake", value);
}

bool spritechat::Options::effectsEnabled() const
{
  return config.value("effects", true).toBool();
}

void spritechat::Options::setEffectsEnabled(bool value)
{
  config.setValue("effects", value);
}

bool spritechat::Options::networkedFrameSfxEnabled() const
{
  return config.value("framenetwork", true).toBool();
}

void spritechat::Options::setNetworkedFrameSfxEnabled(bool value)
{
  config.setValue("framenetwork", value);
}

bool spritechat::Options::slidesEnabled() const
{
  return config.value("slides", true).toBool();
}

void spritechat::Options::setSlidesEnabled(bool value)
{
  config.setValue("slides", value);
}

bool spritechat::Options::colorLogEnabled() const
{
  return config.value("colorlog", true).toBool();
}

void spritechat::Options::setColorLogEnabled(bool value)
{
  config.setValue("colorlog", value);
}

bool spritechat::Options::clearSoundsDropdownOnPlayEnabled() const
{
  return config.value("stickysounds", true).toBool();
}

void spritechat::Options::setClearSoundsDropdownOnPlayEnabled(bool value)
{
  config.setValue("stickysounds", value);
}

bool spritechat::Options::clearEffectsDropdownOnPlayEnabled() const
{
  return config.value("stickyeffects", true).toBool();
}

void spritechat::Options::setClearEffectsDropdownOnPlayEnabled(bool value)
{
  config.setValue("stickyeffects", value);
}

bool spritechat::Options::clearPreOnPlayEnabled() const
{
  return config.value("stickypres", true).toBool();
}

void spritechat::Options::setClearPreOnPlayEnabled(bool value)
{
  config.setValue("stickypres", value);
}

bool spritechat::Options::customChatboxEnabled() const
{
  return config.value("customchat", true).toBool();
}

void spritechat::Options::setCustomChatboxEnabled(bool value)
{
  config.setValue("customchat", value);
}

bool spritechat::Options::characterStickerEnabled() const
{
  return config.value("sticker", true).toBool();
}

void spritechat::Options::setCharacterStickerEnabled(bool value)
{
  config.setValue("sticker", value);
}

bool spritechat::Options::continuousPlaybackEnabled() const
{
  return config.value("continuous_playback", true).toBool();
}

void spritechat::Options::setContinuousPlaybackEnabled(bool value)
{
  config.setValue("continuous_playback", value);
}

bool spritechat::Options::stopMusicOnCategoryEnabled() const
{
  return config.value("category_stop", true).toBool();
}

void spritechat::Options::setStopMusicOnCategoryEnabled(bool value)
{
  config.setValue("category_stop", value);
}

bool spritechat::Options::logToTextFileEnabled() const
{
  return config.value("automatic_logging_enabled", true).toBool();
}

void spritechat::Options::setLogToTextFileEnabled(bool value)
{
  config.setValue("automatic_logging_enabled", value);
}

QString spritechat::Options::subTheme() const
{
  if (settingsSubTheme() == "server" && !m_server_subtheme.isEmpty())
  {
    return m_server_subtheme;
  }
  return settingsSubTheme();
}

QString spritechat::Options::settingsSubTheme() const
{
  return config.value("subtheme", "server").toString();
}

void spritechat::Options::setSettingsSubTheme(QString value)
{
  config.setValue("subtheme", value);
}

QString spritechat::Options::serverSubTheme() const
{
  return m_server_subtheme;
}

void spritechat::Options::setServerSubTheme(QString value)
{
  m_server_subtheme = value;
}

bool spritechat::Options::animatedThemeEnabled() const
{
  return config.value("animated_theme", false).toBool();
}

void spritechat::Options::setAnimatedThemeEnabled(bool value)
{
  config.setValue("animated_theme", value);
}

QStringList spritechat::Options::mountPaths() const
{
  return config.value("mount_paths").value<QStringList>();
}

void spritechat::Options::setMountPaths(QStringList value)
{
  config.setValue("mount_paths", value);
}

bool spritechat::Options::playerCountOptout() const
{
  return config.value("player_count_optout", false).toBool();
}

void spritechat::Options::setPlayerCountOptout(bool value)
{
  config.setValue("player_count_optout", value);
}

bool spritechat::Options::playSelectedSFXOnIdle() const
{
  return config.value("sfx_on_idle", false).toBool();
}

void spritechat::Options::setPlaySelectedSFXOnIdle(bool value)
{
  config.setValue("sfx_on_idle", value);
}

bool spritechat::Options::evidenceDoubleClickEdit() const
{
  return config.value("evidence_double_click", true).toBool();
}

void spritechat::Options::setEvidenceDoubleClickEdit(bool value)
{
  config.setValue("evidence_double_click", value);
}

QString spritechat::Options::alternativeMasterserver() const
{
  return config.value("master", "").toString();
}

void spritechat::Options::setAlternativeMasterserver(QString value)
{
  config.setValue("master", value);
}

QString spritechat::Options::language() const
{
  return config.value("language", QLocale::system().name()).toString();
}

void spritechat::Options::setLanguage(QString value)
{
  config.setValue("language", value);
}

spritechat::RESIZE_MODE spritechat::Options::resizeMode() const
{
  return RESIZE_MODE(config.value("resize_mode", AUTO_RESIZE_MODE).toInt());
}

void spritechat::Options::setResizeMode(RESIZE_MODE value)
{
  config.setValue("resize_mode", value);
}

QStringList spritechat::Options::callwords() const
{
  QStringList l_callwords = config.value("callwords", QStringList{}).toStringList();

  // Please someone explain to me how tf I am supposed to create an empty
  // QStringList using QSetting defaults.
  if (l_callwords.size() == 1 && l_callwords.at(0).isEmpty())
  {
    l_callwords.clear();
  }
  return l_callwords;
}

void spritechat::Options::setCallwords(QStringList value)
{
  config.setValue("callwords", value);
}

QString spritechat::Options::callwordSfx() const
{
  return config.value("callword_sfx").toString();
}

void spritechat::Options::setCallwordSfx(QString value)
{
  config.setValue("callword_sfx", value);
}

QString spritechat::Options::playerlistFormatString() const
{
  return config.value("visuals/playerlist_format", "[{id}] {character} {displayname} {username}").toString();
}

void spritechat::Options::setPlayerlistFormatString(QString value)
{
  config.setValue("visuals/playerlist_format", value);
}

void spritechat::Options::clearConfig()
{
  config.clear();
}

QVector<spritechat::ServerInfo> spritechat::Options::favorites()
{
  QVector<ServerInfo> serverlist;

  auto grouplist = favorite.childGroups();
  { // remove all negative and non-numbers
    auto filtered_grouplist = grouplist;
    for (const QString &group : std::as_const(grouplist))
    {
      bool ok = false;
      const int l_num = group.toInt(&ok);
      if (ok && l_num >= 0)
      {
        continue;
      }
      filtered_grouplist.append(group);
    }
    std::sort(filtered_grouplist.begin(), filtered_grouplist.end(), [](const auto &a, const auto &b) -> bool { return a.toInt() < b.toInt(); });
    grouplist = std::move(filtered_grouplist);
  }

  for (const QString &group : std::as_const(grouplist))
  {
    ServerInfo f_server;
    favorite.beginGroup(group);
    f_server.address = favorite.value("address", "127.0.0.1").toString();
    f_server.port = favorite.value("port", 27016).toInt();
    f_server.name = favorite.value("name", "Missing Name").toString();
    f_server.description = favorite.value("desc", "No description").toString();
    if (favorite.contains("protocol"))
    {
      f_server.protocol = favorite.value("protocol").toString();
    }
    else
    {
      f_server.protocol = "tcp";
    }

    serverlist.append(std::move(f_server));
    favorite.endGroup();
  }

  return serverlist;
}

void spritechat::Options::setFavorites(QVector<ServerInfo> value)
{
  favorite.clear();
  for (int i = 0; i < value.size(); ++i)
  {
    auto fav_server = value.at(i);
    favorite.beginGroup(QString::number(i));
    favorite.setValue("name", fav_server.name);
    favorite.setValue("address", fav_server.address);
    favorite.setValue("port", fav_server.port);
    favorite.setValue("desc", fav_server.description);
    favorite.setValue("protocol", fav_server.protocol);
    favorite.endGroup();
  }
  favorite.sync();
}

void spritechat::Options::removeFavorite(int index)
{
  QVector<ServerInfo> l_favorites = favorites();
  l_favorites.remove(index);
  setFavorites(l_favorites);
}

void spritechat::Options::addFavorite(ServerInfo server)
{
  int index = favorites().size();
  favorite.beginGroup(QString::number(index));
  favorite.setValue("name", server.name);
  favorite.setValue("address", server.address);
  favorite.setValue("port", server.port);
  favorite.setValue("desc", server.description);
  favorite.setValue("protocol", server.protocol);
  favorite.endGroup();
  favorite.sync();
}

void spritechat::Options::updateFavorite(ServerInfo server, int index)
{
  favorite.beginGroup(QString::number(index));
  favorite.setValue("name", server.name);
  favorite.setValue("address", server.address);
  favorite.setValue("port", server.port);
  favorite.setValue("desc", server.description);
  favorite.setValue("protocol", server.protocol);
  favorite.endGroup();
  favorite.sync();
}

QString spritechat::Options::getUIAsset(QString f_asset_name)
{
  QStringList l_paths{":/base/themes/" + Options::getInstance().theme() + "/" + f_asset_name};

  if (subTheme() == "server")
  {
    if (serverSubTheme().isEmpty())
    {
      l_paths.prepend(":/base/themes/" + theme() + "/" + serverSubTheme() + "/" + f_asset_name);
    }
  }
  else
  {
    l_paths.prepend(":/base/themes/" + theme() + "/" + subTheme() + "/" + f_asset_name);
  }

  for (const QString &l_path : std::as_const(l_paths))
  {
    if (QFile::exists(l_path))
    {
      return l_path;
    }
  }
  qWarning() << "Unable to locate ui-asset" << f_asset_name << "in theme" << theme() << "Defaulting to embeeded asset.";
  return QString(":/data/ui/" + f_asset_name);
}

void spritechat::Options::setWindowPosition(QString widget, QPoint position)
{
  config.setValue("windows/position_" + widget, position);
}

std::optional<QPoint> spritechat::Options::windowPosition(QString widget)
{
  QPoint point = config.value("windows/position_" + widget, QPoint()).toPoint();
  if (point.isNull())
  {
    return std::nullopt;
  }
  return std::optional<QPoint>(point);
}

bool spritechat::Options::restoreWindowPositionEnabled() const
{
  return config.value("windows/restore", true).toBool();
}

void spritechat::Options::setRestoreWindowPositionEnabled(bool state)
{
  config.setValue("windows/restore", state);
}
