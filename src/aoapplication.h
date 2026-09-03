#pragma once

#include "ao_track_library.h"
#include "area_registry.h"
#include "asset_lookup.h"
#include "console_logger.h"
#include "core/log.h"
#include "datatypes.h"
#include "evidence_registry.h"
#include "game/emote_cue.h"
#include "inventory_registry.h"
#include "network/master_gateway.h"
#include "network/packet.h"
#include "network/packet_factory.h"
#include "network/packet_router.h"
#include "network/packet_transmitter.h"
#include "network_manager.h"
#include "options.h"
#include "player_registry.h"
#include "protocol/packets/area_packets.h"
#include "protocol/packets/chat_packets.h"
#include "protocol/packets/court_packets.h"
#include "protocol/packets/evidence_packets.h"
#include "protocol/packets/handshake_packets.h"
#include "protocol/packets/ic_packets.h"
#include "protocol/packets/moderation_packets.h"
#include "protocol/packets/music_packets.h"
#include "protocol/packets/roster_packets.h"
#include "protocol/packets/session_packets.h"
#include "protocol/server_info.h"
#include "serverdata.h"
#include "timer.h"
#include "widgets/aooptionsdialog.h"

#include <bass.h>

#include <QColor>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QList>
#include <QObject>
#include <QRect>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QUrl>

namespace spritechat
{
// TODO: fix cyclic dependency
class Lobby;
class Courtroom;

class AOApplication : public QObject, public theory::PacketTransmitter
{
  Q_OBJECT

public:
  explicit AOApplication(const theory::PacketFactory &packet_factory, QObject *parent = nullptr);
  ~AOApplication();

  const theory::PacketFactory &m_packet_factory;

  ConsoleLogger console_logger;
  Lobby *w_lobby = nullptr;
  Courtroom *w_courtroom = nullptr;

  QFont default_font;

  bool is_lobby_constructed();
  void construct_lobby();
  void destruct_lobby();

  bool is_courtroom_constructed();
  void construct_courtroom();
  void destruct_courtroom();

  void call_settings_menu();
  void apply_master_options();

  AssetLookup m_asset_lookup;
  AOTrackLibrary m_track_library;

  ///////////////loading info///////////////////

  VPath get_theme_path(const QString &p_file, const QString &p_theme = QString());
  VPath get_character_path(const QString &p_char, const QString &p_file);
  VPath get_misc_path(const QString &p_misc, const QString &p_file);
  VPath get_sounds_path(const QString &p_file);
  VPath get_music_path(const QString &p_song);
  VPath get_background_path(const QString &p_file);
  VPath get_default_background_path(const QString &p_file);
  VPath get_evidence_path(const QString &p_file);
  QList<VPath> get_asset_paths(const QString &p_element, const QString &p_theme = QString(), const QString &p_subtheme = QString(), const QString &p_default_theme = QString(), const QString &p_misc = QString(), const QString &p_character = QString(), const QString &p_placeholder = QString());
  QString get_asset_path(const QList<VPath> &pathlist);
  QString get_image_path(const QList<VPath> &pathlist, int &index, bool static_image = false);
  QString get_image_path(const QList<VPath> &pathlist, bool static_image = false);
  QString get_sfx_path(const QList<VPath> &pathlist);
  QString get_config_value(const QString &p_identifier, const QString &p_config, const QString &p_theme = QString(), const QString &p_subtheme = QString(), const QString &p_default_theme = QString(), const QString &p_misc = QString());
  QString get_asset(const QString &p_element, const QString &p_theme = QString(), const QString &p_subtheme = QString(), const QString &p_default_theme = QString(), const QString &p_misc = QString(), const QString &p_character = QString(), const QString &p_placeholder = QString());
  QString get_image(const QString &p_element, const QString &p_theme = QString(), const QString &p_subtheme = QString(), const QString &p_default_theme = QString(), const QString &p_misc = QString(), const QString &p_character = QString(), const QString &p_placeholder = QString(), bool static_image = false);
  QString get_sfx(const QString &p_sfx, const QString &p_misc = QString(), const QString &p_character = QString());

  BackgroundPosition get_pos_path(const QString &pos);

  QString get_case_sensitive_path(const QString &p_file);
  QString get_real_path(const VPath &vpath, const QStringList &suffixes = {""});

  QString find_image(const QStringList &p_list);

  ////// Functions for reading and writing files //////

  // returns all of the file's lines in a QStringList
  QStringList get_list_file(const VPath &path);
  QStringList get_list_file(const QString &p_file);

  // Process a file and return its text as a QString
  QString read_file(const QString &filename);

  // Write text to file. make_dir would auto-create the directory if it doesn't
  // exist.
  bool write_to_file(const QString &p_text, const QString &p_file, bool make_dir = false);

  // Append text to the end of the file. make_dir would auto-create the
  // directory if it doesn't exist.
  bool append_to_file(const QString &p_text, const QString &p_file, bool make_dir = false);

  // Returns the value of p_identifier in the design.ini file in p_design_path
  QString read_design_ini(const QString &p_identifier, const VPath &p_design_path);
  QString read_design_ini(const QString &p_identifier, const QString &p_design_path);

  // Returns the coordinates of widget with p_identifier from p_file
  QPoint get_button_spacing(const QString &p_identifier, const QString &p_file);

  // Returns the dimensions of widget with specified identifier from p_file
  pos_size_type get_element_dimensions(const QString &p_identifier, const QString &p_file, const QString &p_misc = QString());

  // Returns the value to you
  QString get_design_element(const QString &p_identifier, const QString &p_file, const QString &p_misc = QString());

  // Returns the color with p_identifier from p_file
  QColor get_color(const QString &p_identifier, const QString &p_file);

  // Returns the markup symbol used for specified p_identifier such as colors
  QString get_chat_markup(const QString &p_identifier, const QString &p_file);

  // Returns the color from the misc folder.
  QColor get_chat_color(const QString &p_identifier, const QString &p_chat);

  // Returns the value with p_identifier from penalty/penalty.ini in the current
  // theme path
  QString get_penalty_value(const QString &p_identifier);

  // Returns the sfx with p_identifier from courtroom_sounds.ini in the current theme path
  QString get_court_sfx(const QString &p_identifier, const QString &p_misc = QString());

  // Figure out if we can opus this or if we should fall back to wav
  QString get_sfx_suffix(const VPath &sound_to_check);

  // Can we use APNG for this? If not, WEBP? If not, GIF? If not, fall back to
  // PNG.
  QString get_image_suffix(const VPath &path_to_check, bool static_image = false);

  // Returns the value of p_search_line within target_tag and terminator_tag
  QString read_char_ini(const QString &p_char, const QString &p_search_line, const QString &target_tag);

  // Returns a QStringList of all key=value definitions on a given tag.
  QStringList read_ini_tags(const VPath &p_file, const QString &target_tag = QString());

  // Returns the text between target_tag and terminator_tag in p_file
  QString get_stylesheet(const QString &p_file);

  // Returns the side of the p_char character from that characters ini file
  QString get_char_side(const QString &p_char);

  // Returns the showname from the ini of p_char
  QString get_showname(const QString &p_char, int p_emote = -1);

  // Returns the category of this character
  QString get_category(const QString &p_char);

  // Returns the value of chat image from the specific p_char's ini file
  QString get_chat(const QString &p_char);

  // Returns the value of chat font from the specific p_char's ini file
  QString get_chat_font(const QString &p_char);

  // Returns the value of chat font size from the specific p_char's ini file
  int get_chat_size(const QString &p_char);

  // Get the theme's effects folder, read it and return the list of filenames in
  // a string
  QStringList get_effects(const QString &p_char);

  // Get the correct effect image
  QString get_effect(const QString &effect, const QString &p_char, const QString &p_folder);

  // Return p_property of fx_name. If p_property is "sound", return
  // the value associated with fx_name, otherwise use fx_name + '_' + p_property.
  QString get_effect_property(const QString &fx_name, const QString &p_char, const QString &p_folder, const QString &p_property);

  // Returns the custom realisation used by the character.
  QString get_custom_realization(const QString &p_char);

  // Returns whether the given pos is a judge position
  bool get_pos_is_judge(const QString &p_pos);

  /**
   * @brief Returns the duration of the transition animation between the two
   * given positions, if it exists
   */
  int get_pos_transition_duration(const QString &old_pos, const QString &new_pos);

  // Returns the total amount of emotes of p_char
  int get_emote_number(const QString &p_char);

  // Returns the emote comment of p_char's p_emote
  QString get_emote_comment(const QString &p_char, int p_emote);

  // Returns the base name of p_char's p_emote
  QString get_emote(const QString &p_char, int p_emote);

  // Returns the preanimation name of p_char's p_emote
  QString get_pre_emote(const QString &p_char, int p_emote);

  // Returns the sfx of p_char's p_emote
  QString get_sfx_name(const QString &p_char, int p_emote);

  // Returns if the sfx is defined as looping in char.ini
  QString get_sfx_looping(const QString &p_char, int p_emote);

  // Returns the frame cues for an emote as defined in the char.ini
  QList<theory::EmoteCue> get_emote_cues(const QString &p_char, const QString &p_emote);

  // Returns the sfx delay in milliseconds
  int get_sfx_delay(const QString &p_char, int p_emote);

  // Returns the modifier for p_char's p_emote
  int get_emote_mod(const QString &p_char, int p_emote);

  // Returns the desk modifier for p_char's p_emote
  int get_desk_mod(const QString &p_char, int p_emote);

  // Returns p_char's blipname by reading char.ini for blips (previously called "gender")
  QString get_blipname(const QString &p_char, int p_emote = -1);

  // Returns p_blipname's sound(path) to play in the client
  QString get_blips(const QString &p_blipname);

  // Get a property of a given emote, or get it from "options" if emote doesn't have it
  QString get_emote_property(const QString &p_char, const QString &p_emote, const QString &p_property);

  // Return a transformation mode from a string ("smooth" for smooth, anything else for fast)
  RESIZE_MODE get_scaling(const QString &p_scaling);

  // Returns the scaling type for p_miscname
  RESIZE_MODE get_misc_scaling(const QString &p_miscname);

  // ======
  // These are all casing-related settings.
  // ======

  // Currently defined subtheme
  QString subtheme;

  const QString default_theme = "default"; // don't change this!!! don't do it!!!

  bool pointExistsOnScreen(QPoint point);
  void centerOrMoveWidgetOnPrimaryScreen(QWidget *widget);

  void initBASS();
  static void load_bass_plugins();
  static void CALLBACK BASSreset(HSTREAM handle, DWORD channel, DWORD data, void *user);
  static void doBASSreset();

  MasterGateway *master_gateway;

  // The name of the currently connected server.
  QString server_name;

  // The file name of the log file in base/logs.
  QString log_filename;

  /// Stores everything related to the server the client is connected to, if
  /// any.
  ServerData m_server_data;

  // client ID. Not useful, to be removed eventually
  theory::PlayerId m_player_id = theory::NoPlayerId;

private:
  theory::Log m_log;

  /**
   * Connection, Instance, Session
   */

  NetworkManager *net_manager;
  ServerBookmark m_server;
  theory::ServerInfo m_server_info;
  theory::PacketRouter m_router;

  QString window_title;

  QTimer *m_keepalive_timer;

  void shipPacket(const theory::Packet &packet) override;
  void register_packet_routes();

  void connect_to_server(const ServerBookmark &server, const theory::ServerInfo &info);
  void reconnect_to_server();

  AreaRegistry m_area_registry;
  PlayerRegistry m_player_registry;
  InventoryRegistry m_inventory_registry;
  EvidenceRegistry m_evidence_registry;
  QList<Timer *> m_timers;

  Timer *timer(theory::TimerId id) const;
  void reset_server_instance();

  bool m_session_active = false;
  bool m_recovered_session = false;
  QHash<QUrl, QString> m_tokens;

  void start_session();
  void stop_session();
  void drop_session();

  void process(const theory::CharacterListPacket &packet);
  void process(const theory::MusicListPacket &packet);

  void process(const theory::CharacterAcceptedPacket &packet);

  void process(const theory::BackgroundPacket &packet);
  void process(const theory::SetPositionPacket &packet);
  void process(const theory::AreaRecordPacket &packet);
  void process(const theory::AreaUpdatePacket &packet);
  void process(const theory::SubthemePacket &packet);
  void process(const theory::TimerPacket &packet);

  void process(const theory::IcMessagePacket &packet);
  void process(const theory::OocMessagePacket &packet);
  void process(const theory::ServerMessagePacket &packet);
  void process(const theory::MusicChangedPacket &packet);
  void process(const theory::PenaltyPacket &packet);
  void process(const theory::SplashPacket &packet);

  void process(const theory::PlayerRecordPacket &packet);
  void process(const theory::PlayerUpdatePacket &packet);

  void process(const theory::InventoryRecordPacket &packet);
  void process(const theory::InventoryUpdatePacket &packet);
  void process(const theory::EvidenceRecordPacket &packet);
  void process(const theory::EvidenceUpdatePacket &packet);

  void process(const theory::ModCallNoticePacket &packet);
  void process(const theory::AuthStatePacket &packet);
  void process(const theory::GameErrorPacket &packet);
  void process(const theory::ErrorPacket &packet);

  void process(const theory::SessionGrantPacket &packet);
  void process(const theory::WelcomePacket &packet);

private Q_SLOTS:
  void process_pending_packets();

  void handle_network_status(NetworkManager::Status status);
  void handle_network_error(const theory::CargoError &error);
};
} // namespace spritechat
