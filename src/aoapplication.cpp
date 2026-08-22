#include "aoapplication.h"

#include "core/logging.h"
#include "courtroom.h"
#include "hardware_functions.h"
#include "lobby.h"
#include "network_manager.h"
#include "options.h"
#include "spritechat_log.h"
#include "widgets/aooptionsdialog.h"

#include "protocol/packets/handshake_packets.h"
#include "protocol/protocol_info.h"

#include <QDateTime>
#include <QRegularExpression>

spritechat::AOApplication::AOApplication(const theory::PacketFactory &packet_factory, QObject *parent)
    : QObject(parent)
    , m_packet_factory{packet_factory}
{
  register_packet_routes();

  for (theory::TimerId id = 0; id < theory::TimerCount; id++)
  {
    m_timers.append(new Timer(id, this));
  }

  net_manager = new NetworkManager(m_packet_factory, this);
  connect(net_manager, &NetworkManager::statusChanged, this, &AOApplication::handle_network_status);
  connect(net_manager, &NetworkManager::errorOccurred, this, &AOApplication::handle_network_error);
  connect(net_manager, &NetworkManager::pendingPacketAvailable, this, &AOApplication::process_pending_packets);
  connect(net_manager, &NetworkManager::pong, this, [this](quint64 elapsedTime) {
      w_courtroom->setWindowTitle(QStringLiteral("%1 (%2 ms)").arg(window_title).arg(elapsedTime));
  });

  m_keepalive_timer = new QTimer(this);
  m_keepalive_timer->setInterval(45000);
  connect(m_keepalive_timer, &QTimer::timeout, net_manager, &NetworkManager::ping);

  master_gateway = new MasterGateway(this);
  apply_master_options();

  theory::Log::add(console_logger);
}

spritechat::AOApplication::~AOApplication()
{
  destruct_lobby();
  destruct_courtroom();
  theory::Log::remove(console_logger);
}

bool spritechat::AOApplication::is_lobby_constructed()
{
  return w_lobby;
}

void spritechat::AOApplication::construct_lobby()
{
  if (is_lobby_constructed())
  {
    return;
  }

  w_lobby = new Lobby(this, *net_manager, *master_gateway);

  connect(w_lobby, &Lobby::connection_requested, this, &AOApplication::connect_to_server);

  centerOrMoveWidgetOnPrimaryScreen(w_lobby);

  w_lobby->show();
}

void spritechat::AOApplication::destruct_lobby()
{
  if (!is_lobby_constructed())
  {
    return;
  }

  Options::getInstance().setWindowPosition(w_lobby->objectName(), w_lobby->pos());

  delete w_lobby;
  w_lobby = nullptr;
}

bool spritechat::AOApplication::is_courtroom_constructed()
{
  return w_courtroom;
}

void spritechat::AOApplication::construct_courtroom()
{
  if (is_courtroom_constructed())
  {
    zWarning(log::ui) << "courtroom was attempted constructed when it already exists";
    return;
  }

  w_courtroom = new Courtroom(this, m_area_registry, m_player_registry, m_timers, *net_manager);

  connect(w_courtroom, &Courtroom::aboutToClose, this, [this] {
    drop_session();
    createAndShipPacket<theory::GoodbyePacket>();
    net_manager->disconnectFromServer();
  });

  w_courtroom->setWindowTitle(window_title);

  m_keepalive_timer->start();

  centerOrMoveWidgetOnPrimaryScreen(w_courtroom);
}

void spritechat::AOApplication::destruct_courtroom()
{
  if (!w_courtroom)
  {
    return;
  }

  m_keepalive_timer->stop();

  Options::getInstance().setWindowPosition(w_courtroom->objectName(), w_courtroom->pos());

  delete w_courtroom;
  w_courtroom = nullptr;
  m_asset_lookup.setCurrentBackground(QString());
}

void spritechat::AOApplication::reset_server_instance()
{
  m_area_registry.clear();
  m_player_registry.clear();
  for (Timer *l_timer : m_timers)
  {
    l_timer->reset();
  }
}

void spritechat::AOApplication::call_settings_menu()
{
  AOOptionsDialog *l_dialog = new AOOptionsDialog(this);

  if (is_courtroom_constructed())
  {
    connect(l_dialog, &AOOptionsDialog::reloadThemeRequest, w_courtroom, &Courtroom::on_reload_theme_clicked);
  }

  if (is_lobby_constructed())
  {}
  l_dialog->exec();

  apply_master_options();

  if (is_courtroom_constructed())
  {
    w_courtroom->playerList()->reloadPlayers();
  }

  delete l_dialog;
}

void spritechat::AOApplication::apply_master_options()
{
  master_gateway->setUrl(QUrl(Options::getInstance().masterServerUrl()));
  master_gateway->setTelemetryEnabled(!Options::getInstance().playerCountOptout());
  master_gateway->setLanguage(Options::getInstance().language());
}

spritechat::VPath spritechat::AOApplication::get_theme_path(const QString &p_file, const QString &p_theme)
{
  return m_asset_lookup.get_theme_path(p_file, p_theme);
}

spritechat::VPath spritechat::AOApplication::get_character_path(const QString &p_char, const QString &p_file)
{
  return m_asset_lookup.get_character_path(p_char, p_file);
}

spritechat::VPath spritechat::AOApplication::get_misc_path(const QString &p_misc, const QString &p_file)
{
  return m_asset_lookup.get_misc_path(p_misc, p_file);
}

spritechat::VPath spritechat::AOApplication::get_sounds_path(const QString &p_file)
{
  return m_asset_lookup.get_sounds_path(p_file);
}

spritechat::VPath spritechat::AOApplication::get_music_path(const QString &p_song)
{
  return m_asset_lookup.get_music_path(p_song);
}

spritechat::VPath spritechat::AOApplication::get_background_path(const QString &p_file)
{
  return m_asset_lookup.get_background_path(p_file);
}

#if (defined(_WIN32) || defined(_WIN64))
spritechat::VPath spritechat::AOApplication::get_default_background_path(const QString &p_file)
{
  return m_asset_lookup.get_default_background_path(p_file);
}
#elif defined __APPLE__
void spritechat::AOApplication::load_bass_plugins()
{
  BASS_PluginLoad("libbassopus.dylib", 0);
}
#elif (defined(LINUX) || defined(__linux__))
void spritechat::AOApplication::load_bass_plugins()
{
  BASS_PluginLoad("libbassopus.so", 0);
}
#else
#error This operating system is unsupported for BASS plugins.
#endif

spritechat::VPath spritechat::AOApplication::get_evidence_path(const QString &p_file)
{
  return m_asset_lookup.get_evidence_path(p_file);
}

QList<spritechat::VPath> spritechat::AOApplication::get_asset_paths(const QString &p_element, const QString &p_theme, const QString &p_subtheme, const QString &p_default_theme, const QString &p_misc, const QString &p_character, const QString &p_placeholder)
{
  return m_asset_lookup.get_asset_paths(p_element, p_theme, p_subtheme, p_default_theme, p_misc, p_character, p_placeholder);
}

QString spritechat::AOApplication::get_asset_path(const QList<VPath> &pathlist)
{
  return m_asset_lookup.get_asset_path(pathlist);
}

QString spritechat::AOApplication::get_image_path(const QList<VPath> &pathlist, int &index, bool static_image)
{
  return m_asset_lookup.get_image_path(pathlist, index, static_image);
}

QString spritechat::AOApplication::get_image_path(const QList<VPath> &pathlist, bool static_image)
{
  return m_asset_lookup.get_image_path(pathlist, static_image);
}

QString spritechat::AOApplication::get_sfx_path(const QList<VPath> &pathlist)
{
  return m_asset_lookup.get_sfx_path(pathlist);
}

QString spritechat::AOApplication::get_config_value(const QString &p_identifier, const QString &p_config, const QString &p_theme, const QString &p_subtheme, const QString &p_default_theme, const QString &p_misc)
{
  return m_asset_lookup.get_config_value(p_identifier, p_config, p_theme, p_subtheme, p_default_theme, p_misc);
}

QString spritechat::AOApplication::get_asset(const QString &p_element, const QString &p_theme, const QString &p_subtheme, const QString &p_default_theme, const QString &p_misc, const QString &p_character, const QString &p_placeholder)
{
  return m_asset_lookup.get_asset(p_element, p_theme, p_subtheme, p_default_theme, p_misc, p_character, p_placeholder);
}

QString spritechat::AOApplication::get_image(const QString &p_element, const QString &p_theme, const QString &p_subtheme, const QString &p_default_theme, const QString &p_misc, const QString &p_character, const QString &p_placeholder, bool static_image)
{
  return m_asset_lookup.get_image(p_element, p_theme, p_subtheme, p_default_theme, p_misc, p_character, p_placeholder, static_image);
}

QString spritechat::AOApplication::get_sfx(const QString &p_sfx, const QString &p_misc, const QString &p_character)
{
  return m_asset_lookup.get_sfx(p_sfx, p_misc, p_character);
}

spritechat::BackgroundPosition spritechat::AOApplication::get_pos_path(const QString &pos)
{
  return m_asset_lookup.get_pos_path(pos);
}

QString spritechat::AOApplication::get_case_sensitive_path(const QString &p_file)
{
  return m_asset_lookup.get_case_sensitive_path(p_file);
}

QString spritechat::AOApplication::get_real_path(const VPath &vpath, const QStringList &suffixes)
{
  return m_asset_lookup.get_real_path(vpath, suffixes);
}

QString spritechat::AOApplication::find_image(const QStringList &p_list)
{
  QString image_path;
  for (const QString &path : p_list)
  {
    if (file_exists(path))
    {
      image_path = path;
      break;
    }
  }
  return image_path;
}

QStringList spritechat::AOApplication::get_list_file(const VPath &path)
{
  return m_asset_lookup.get_list_file(path);
}

QStringList spritechat::AOApplication::get_list_file(const QString &p_file)
{
  return m_asset_lookup.get_list_file(p_file);
}

QString spritechat::AOApplication::read_file(const QString &filename)
{
  return m_asset_lookup.read_file(filename);
}

bool spritechat::AOApplication::write_to_file(const QString &p_text, const QString &p_file, bool make_dir)
{
  return m_asset_lookup.write_to_file(p_text, p_file, make_dir);
}

bool spritechat::AOApplication::append_to_file(const QString &p_text, const QString &p_file, bool make_dir)
{
  return m_asset_lookup.append_to_file(p_text, p_file, make_dir);
}

QString spritechat::AOApplication::read_design_ini(const QString &p_identifier, const VPath &p_design_path)
{
  return m_asset_lookup.read_design_ini(p_identifier, p_design_path);
}

QString spritechat::AOApplication::read_design_ini(const QString &p_identifier, const QString &p_design_path)
{
  return m_asset_lookup.read_design_ini(p_identifier, p_design_path);
}

QPoint spritechat::AOApplication::get_button_spacing(const QString &p_identifier, const QString &p_file)
{
  return m_asset_lookup.get_button_spacing(p_identifier, p_file);
}

spritechat::pos_size_type spritechat::AOApplication::get_element_dimensions(const QString &p_identifier, const QString &p_file, const QString &p_misc)
{
  return m_asset_lookup.get_element_dimensions(p_identifier, p_file, p_misc);
}

QString spritechat::AOApplication::get_design_element(const QString &p_identifier, const QString &p_file, const QString &p_misc)
{
  return m_asset_lookup.get_design_element(p_identifier, p_file, p_misc);
}

QColor spritechat::AOApplication::get_color(const QString &p_identifier, const QString &p_file)
{
  return m_asset_lookup.get_color(p_identifier, p_file);
}

QString spritechat::AOApplication::get_chat_markup(const QString &p_identifier, const QString &p_file)
{
  return m_asset_lookup.get_chat_markup(p_identifier, p_file);
}

QColor spritechat::AOApplication::get_chat_color(const QString &p_identifier, const QString &p_chat)
{
  return m_asset_lookup.get_chat_color(p_identifier, p_chat);
}

QString spritechat::AOApplication::get_penalty_value(const QString &p_identifier)
{
  return m_asset_lookup.get_penalty_value(p_identifier);
}

QString spritechat::AOApplication::get_court_sfx(const QString &p_identifier, const QString &p_misc)
{
  return m_asset_lookup.get_court_sfx(p_identifier, p_misc);
}

QString spritechat::AOApplication::get_sfx_suffix(const VPath &sound_to_check)
{
  return m_asset_lookup.get_sfx_suffix(sound_to_check);
}

spritechat::AOMusicTrack spritechat::AOApplication::get_music_track(const QString &p_song)
{
  return m_asset_lookup.get_music_track(p_song);
}

QString spritechat::AOApplication::get_image_suffix(const VPath &path_to_check, bool static_image)
{
  return m_asset_lookup.get_image_suffix(path_to_check, static_image);
}

QString spritechat::AOApplication::read_char_ini(const QString &p_char, const QString &p_search_line, const QString &target_tag)
{
  return m_asset_lookup.read_char_ini(p_char, p_search_line, target_tag);
}

QStringList spritechat::AOApplication::read_ini_tags(const VPath &p_file, const QString &target_tag)
{
  return m_asset_lookup.read_ini_tags(p_file, target_tag);
}

QString spritechat::AOApplication::get_stylesheet(const QString &p_file)
{
  return m_asset_lookup.get_stylesheet(p_file);
}

QString spritechat::AOApplication::get_char_side(const QString &p_char)
{
  return m_asset_lookup.get_char_side(p_char);
}

QString spritechat::AOApplication::get_showname(const QString &p_char, int p_emote)
{
  return m_asset_lookup.get_showname(p_char, p_emote);
}

QString spritechat::AOApplication::get_category(const QString &p_char)
{
  return m_asset_lookup.get_category(p_char);
}

QString spritechat::AOApplication::get_chat(const QString &p_char)
{
  return m_asset_lookup.get_chat(p_char);
}

QString spritechat::AOApplication::get_chat_font(const QString &p_char)
{
  return m_asset_lookup.get_chat_font(p_char);
}

int spritechat::AOApplication::get_chat_size(const QString &p_char)
{
  return m_asset_lookup.get_chat_size(p_char);
}

QStringList spritechat::AOApplication::get_effects(const QString &p_char)
{
  return m_asset_lookup.get_effects(p_char);
}

QString spritechat::AOApplication::get_effect(const QString &effect, const QString &p_char, const QString &p_folder)
{
  return m_asset_lookup.get_effect(effect, p_char, p_folder);
}

QString spritechat::AOApplication::get_effect_property(const QString &fx_name, const QString &p_char, const QString &p_folder, const QString &p_property)
{
  return m_asset_lookup.get_effect_property(fx_name, p_char, p_folder, p_property);
}

QString spritechat::AOApplication::get_custom_realization(const QString &p_char)
{
  return m_asset_lookup.get_custom_realization(p_char);
}

bool spritechat::AOApplication::get_pos_is_judge(const QString &p_pos)
{
  return m_asset_lookup.get_pos_is_judge(p_pos);
}

int spritechat::AOApplication::get_pos_transition_duration(const QString &old_pos, const QString &new_pos)
{
  return m_asset_lookup.get_pos_transition_duration(old_pos, new_pos);
}

int spritechat::AOApplication::get_emote_number(const QString &p_char)
{
  return m_asset_lookup.get_emote_number(p_char);
}

QString spritechat::AOApplication::get_emote_comment(const QString &p_char, int p_emote)
{
  return m_asset_lookup.get_emote_comment(p_char, p_emote);
}

QString spritechat::AOApplication::get_emote(const QString &p_char, int p_emote)
{
  return m_asset_lookup.get_emote(p_char, p_emote);
}

QString spritechat::AOApplication::get_pre_emote(const QString &p_char, int p_emote)
{
  return m_asset_lookup.get_pre_emote(p_char, p_emote);
}

QString spritechat::AOApplication::get_sfx_name(const QString &p_char, int p_emote)
{
  return m_asset_lookup.get_sfx_name(p_char, p_emote);
}

QString spritechat::AOApplication::get_sfx_looping(const QString &p_char, int p_emote)
{
  return m_asset_lookup.get_sfx_looping(p_char, p_emote);
}

QList<theory::EmoteCue> spritechat::AOApplication::get_emote_cues(const QString &p_char, const QString &p_emote)
{
  return m_asset_lookup.get_emote_cues(p_char, p_emote);
}

int spritechat::AOApplication::get_sfx_delay(const QString &p_char, int p_emote)
{
  return m_asset_lookup.get_sfx_delay(p_char, p_emote);
}

int spritechat::AOApplication::get_emote_mod(const QString &p_char, int p_emote)
{
  return m_asset_lookup.get_emote_mod(p_char, p_emote);
}

int spritechat::AOApplication::get_desk_mod(const QString &p_char, int p_emote)
{
  return m_asset_lookup.get_desk_mod(p_char, p_emote);
}

QString spritechat::AOApplication::get_blipname(const QString &p_char, int p_emote)
{
  return m_asset_lookup.get_blipname(p_char, p_emote);
}

QString spritechat::AOApplication::get_blips(const QString &p_blipname)
{
  return m_asset_lookup.get_blips(p_blipname);
}

QString spritechat::AOApplication::get_emote_property(const QString &p_char, const QString &p_emote, const QString &p_property)
{
  return m_asset_lookup.get_emote_property(p_char, p_emote, p_property);
}

spritechat::RESIZE_MODE spritechat::AOApplication::get_scaling(const QString &p_scaling)
{
  return m_asset_lookup.get_scaling(p_scaling);
}

spritechat::RESIZE_MODE spritechat::AOApplication::get_misc_scaling(const QString &p_miscname)
{
  return m_asset_lookup.get_misc_scaling(p_miscname);
}

bool spritechat::AOApplication::pointExistsOnScreen(QPoint point)
{
  for (QScreen *screen : QApplication::screens())
  {
    if (screen->availableGeometry().contains(point))
    {
      return true;
    }
  }
  return false;
}

void spritechat::AOApplication::centerOrMoveWidgetOnPrimaryScreen(QWidget *widget)
{
  auto point = Options::getInstance().windowPosition(widget->objectName());
  if (!Options::getInstance().restoreWindowPositionEnabled() || !point || !pointExistsOnScreen(point.value()))
  {
    QRect geometry = QGuiApplication::primaryScreen()->geometry();
    int x = (geometry.width() - widget->width()) / 2;
    int y = (geometry.height() - widget->height()) / 2;
    widget->move(x, y);
  }
  else
  {
    widget->move(point->x(), point->y());
  }
}

void spritechat::AOApplication::initBASS()
{
  BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, 1);
  BASS_Free();
  // Change the default audio output device to be the one the user has given
  // in his config.ini file for now.
  unsigned int a = 0;
  BASS_DEVICEINFO info;

  if (Options::getInstance().audioOutputDevice() == "default")
  {
    BASS_Init(-1, 48000, BASS_DEVICE_LATENCY, nullptr, nullptr);
    load_bass_plugins();
  }
  else
  {
    for (a = 0; BASS_GetDeviceInfo(a, &info); a++)
    {
      if (Options::getInstance().audioOutputDevice() == info.name)
      {
        BASS_SetDevice(a);
        BASS_Init(static_cast<int>(a), 48000, BASS_DEVICE_LATENCY, nullptr, nullptr);
        load_bass_plugins();
        zInfo(log::audio) << info.name << "was set as the default audio output device.";
        return;
      }
    }
    BASS_Init(-1, 48000, BASS_DEVICE_LATENCY, nullptr, nullptr);
    load_bass_plugins();
  }
}

void spritechat::AOApplication::load_bass_plugins()
{
  BASS_PluginLoad("bassopus.dll", 0);
}

// Callback for when BASS device is lost
// Only actually used for music syncs
void CALLBACK spritechat::AOApplication::BASSreset(HSTREAM handle, DWORD channel, DWORD data, void *user)
{
  Q_UNUSED(handle);
  Q_UNUSED(channel);
  Q_UNUSED(data);
  Q_UNUSED(user);
  doBASSreset();
}

void spritechat::AOApplication::doBASSreset()
{
  BASS_Free();
  BASS_Init(-1, 48000, BASS_DEVICE_LATENCY, nullptr, nullptr);
  load_bass_plugins();
}
