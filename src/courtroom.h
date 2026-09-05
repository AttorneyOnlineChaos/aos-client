#pragma once

#include "animationlayer.h"
#include "ao_line_edit.h"
#include "ao_track_library.h"
#include "aoapplication.h"
#include "aoblipplayer.h"
#include "aobutton.h"
#include "aocharbutton.h"
#include "aoclocklabel.h"
#include "aoemotebutton.h"
#include "aoemotepreview.h"
#include "aoevidencedisplay.h"
#include "aoimage.h"
#include "aomusicplayer.h"
#include "aosfxplayer.h"
#include "aotextarea.h"
#include "aotextboxwidgets.h"
#include "area_registry.h"
#include "chatlogpiece.h"
#include "datatypes.h"
#include "debug_functions.h"
#include "eventfilters.h"
#include "evidence_registry.h"
#include "file_functions.h"
#include "game/evidence.h"
#include "game/music.h"
#include "hardware_functions.h"
#include "inventory_registry.h"
#include "lobby.h"
#include "network/packet_transmitter.h"
#include "player_registry.h"
#include "protocol/packets/evidence_packets.h"
#include "protocol/packets/ic_packets.h"
#include "protocol/packets/music_packets.h"
#include "screenslidetimer.h"
#include "scrolltext.h"
#include "server_settings_handle.h"
#include "timer.h"
#include "widgets/aooptionsdialog.h"
#include "widgets/evidence_panel.h"
#include "widgets/navigable_grid.h"
#include "widgets/playerlistwidget.h"
#include "widgets/mousewheel_grid_navigator.h"

#include <QBrush>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QElapsedTimer>
#include <QFont>
#include <QFuture>
#include <QHash>
#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QParallelAnimationGroup>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSet>
#include <QSlider>
#include <QSpinBox>
#include <QTextBoundaryFinder>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTreeWidget>

#include <algorithm>
#include <optional>
#include <stack>

namespace spritechat
{
class Courtroom : public QMainWindow
{
  Q_OBJECT

public:
  explicit Courtroom(AOApplication *p_ao_app, AreaRegistry &p_area_registry, PlayerRegistry &p_player_registry, InventoryRegistry &p_inventory_registry, EvidenceRegistry &p_evidence_registry, ServerSettingsHandle &p_server_settings, const QList<Timer *> &p_timers, theory::PacketTransmitter &p_transport, const AOTrackLibrary &p_track_library);
  ~Courtroom();

  void update_audio_volume();

  void set_characters(const QList<theory::CharacterId> &characters);

  void set_music(const QList<theory::MusicPlaylist> &f_playlists);
  std::optional<theory::MusicTrack> find_track(const QString &f_file_name) const;

  PlayerListWidget *playerList();

  void update_message_capacity();
  void update_mousewheel_direction();
  void apply_server_settings();

  void refresh_area(theory::AreaId n_area);

  void character_loading_finished();

  void set_courtroom_size();

  // sets position of widgets based on theme ini files
  void set_widgets();

  // sets font size based on theme ini files
  void set_font(QWidget *widget, const QString &class_name, const QString &p_identifier, const QString &p_char = QString(), QString font_name = QString(), int f_pointsize = 0);

  // Get the properly constructed font
  QFont get_qfont(QString font_name, int f_pointsize, bool antialias = true);

  // actual operation of setting the font on a widget
  void set_qfont(QWidget *widget, QString class_name, QFont font, QColor f_color = Qt::black, bool bold = false, bool outlined = false, QColor outline_color = QColor(0, 0, 0), int outline_width = 1);

  // helper function that calls above function on the relevant widgets
  void set_fonts(const QString &p_char = QString());

  // sets dropdown menu stylesheet
  void set_stylesheet(QWidget *widget);

  // helper funciton that call above function on the relevant widgets
  void set_stylesheets();

  // reads theme and sets size and pos based on the identifier (using p_misc if provided)
  bool set_size_and_pos(QWidget *p_widget, const QString &p_identifier, const QString &p_misc = QString());

  void refresh_taken_chars();

  // sets the current background to argument. also does some checks to see if
  // it's a legacy bg
  void set_background(const QString &p_background, bool display = false);

  // sets the local character pos/side to use.
  void set_side(const QString &p_side);

  // sets the pos dropdown
  void set_pos_dropdown(const QStringList &pos_dropdowns);

  void enter_char_select();

  // sets the local mute list based on characters available on the server
  void set_mute_list();

  // Sets the local pair list based on the characters available on the server.
  void set_pair_list();

  // sets desk and bg based on pos in chatmessage
  void set_scene(bool show_desk, const QString &f_side);

  // sets p_layer according to the message offsets, only a function bc it's
  // used with desk_mod 4 and 5
  void set_self_offset(int offset_x, int offset_y, AnimationLayer *p_layer);

  // takes in serverD-formatted IP list as prints a converted version to server
  // OOC admittedly poorly named
  void set_ip_list(QString p_list);

  theory::CharacterId get_character_id();

  // cid = character id, returns the cid of the currently selected character
  QString get_current_char();
  QString get_current_background();

  QString default_side();
  QString current_or_default_side();

  // updates character to p_cid and updates necessary ui elements
  void update_character(const theory::CharacterId &p_cid);

  // properly sets up some varibles: resets user state
  void enter_courtroom();

  // helper function that populates ui_music_list with the contents of
  // music_list
  void list_music();
  void list_areas();

  // Debug log (formerly master server chat log)
  void debug_message_handler(QtMsgType type, const QString &msg);

  // OOC chat log
  void append_server_chatmessage(const QString &p_name, const QString &p_message, const QString &p_color);

  void unpack_chatmessage(theory::IcMessagePacket packet);

  // Log the message contents and information such as evidence presenting etc. into the log file, the IC log, or both.
  void log_chatmessage();

  QString current_showname();

  // Log the message contents and information such as evidence presenting etc. into the IC logs
  void handle_callwords();

  // Handle the objection logic, if it's interrupting the currently parsing message.
  // Returns true if this message has an objection, otherwise returns false. The result decides when to call handle_ic_message()
  bool handle_objection();

  // Display the evidence image box when presenting evidence in IC
  void display_evidence_image();

  // Handle the stuff that comes when the character appears on screen and starts animating (preanims etc.)
  void handle_ic_message();

  // Start the logic for doing a courtroom pan slide
  void do_transition(theory::DeskMod desk_mod, const QString &oldPosId, const QString &new_pos);

  // Display the character.
  void display_character();

  // Display the character's pair if present.
  void display_pair_character();

  // Handle the emote mode value and proceed through the logic accordingly.
  void handle_emote_mod(theory::EmoteMode emote_mode, bool p_immediate);

  // Initialize the chatbox image, showname shenanigans, custom chatboxes, etc.
  void initialize_chatbox();

  // Finally start displaying the chatbox we initialized, display the evidence, and play the talking or idle emote for the character.
  // Callwords are also handled here.
  void handle_ic_speaking();

  // This function filters out the common CC inline text trickery, for appending
  // to the IC chatlog.
  QString filter_ic_text(QString p_text, bool colorize = false, int pos = -1, int default_color = 0);

  void log_ic_text(const QString &p_name, const QString &p_showname, const QString &p_message, const QString &p_action = QString(), int p_color = 0, bool p_selfname = false);

  // adds text to the IC chatlog. p_name first as bold then p_text then a newlin
  // this function keeps the chatlog scrolled to the top unless there's text
  // selected
  // or the user isn't already scrolled to the top
  void append_ic_text(const QString &p_text, const QString &p_name = QString(), const QString &p_char = QString(), const QString &action = QString(), int color = 0, bool selfname = false, const QDateTime &timestamp = QDateTime::currentDateTime());

  // prints who played the song to IC chat and plays said song(if found on
  // local filesystem)
  void handle_song(const theory::MusicChangedPacket &packet);

  void play_preanim(bool immediate);

  // plays the witness testimony or cross examination animation based on
  // argument
  void handle_wtce(const theory::SplashPacket &packet);

  // sets the hp bar of defense(p_bar 1) or pro(p_bar 2)
  // state is an number between 0 and 10 inclusive
  void set_hp_bar(theory::HealthBar p_bar, int p_state);

  // Toggles the judge buttons, whether they should appear or not.
  void show_judge_controls(bool visible);

  // Truncates text so it fits within theme-specified boundaries and sets the tooltip to the full string
  void truncate_label_text(QWidget *p_widget, const QString &p_identifier);

  void on_authentication_state_received(int p_state);

  void set_judge_buttons();

Q_SIGNALS:
  void aboutToClose();

protected:
  virtual void closeEvent(QCloseEvent *event) override;

private:
  AOApplication *ao_app;

  // Percentage of audio that is suppressed when client is not in focus
  int suppress_audio = 0;

  int m_courtroom_width = 714;
  int m_courtroom_height = 668;

  int m_viewport_x = 0;
  int m_viewport_y = 0;

  int m_viewport_width = 256;
  int m_viewport_height = 192;

  int maximumMessages = 0;

  QParallelAnimationGroup *m_screenshake_anim_group;

  ScreenSlideTimer *m_screenslide_timer;

  bool next_character_is_not_special = false; // If true, write the
                                              // next character as it is.

  bool message_is_centered = false;

  int current_display_speed = 3;
  int text_crawl = 40;
  double message_display_mult[7] = {0, 0.25, 0.65, 1, 1.25, 1.75, 2.25};

  // The player this user wants to appear alongside with.
  theory::PlayerId other_player_id = theory::NoPlayerId;

  // The horizontal offset this user has given if they want to appear alongside someone.
  int char_offset = 0;

  // The vertical offset this user has given.
  int char_vert_offset = 0;

  // 0 = in front, 1 = behind
  int pair_order = 0;

  QList<theory::CharacterId> char_list;
  QSet<theory::CharacterId> taken_chars;
  QList<theory::MusicPlaylist> music_list;
  AreaRegistry &area_registry;
  PlayerRegistry &player_registry;
  InventoryRegistry &inventory_registry;
  EvidenceRegistry &evidence_registry;
  ServerSettingsHandle &server_settings;
  const QList<Timer *> &timers;
  theory::PacketTransmitter &transport;
  const AOTrackLibrary &track_library;

  QList<ChatLogPiece> ic_chatlog_history;
  QString last_ic_message;
  QString shown_motd;

  // determines how fast messages tick onto screen
  QTimer *chat_tick_timer;

  // int chat_tick_interval = 60;
  // which tick position(character in chat message) we are at
  int tick_pos = 0;
  // the actual document tick pos we gotta worry about for making the text
  // scroll better
  int real_tick_pos = 0;
  // used to determine how often blips sound
  int blip_ticker = 0;
  int blip_rate = 2;
  int rainbow_counter = 0;
  bool rainbow_appended = false;
  bool blank_blip = false;
  bool chatbox_always_show = false;

  // Used for getting the current maximum blocks allowed in the IC chatlog.
  int log_maximum_blocks = 0;

  // True, if the log should go downwards.
  bool log_goes_downwards = true;

  // True, if log should display colors.
  bool log_colors = true;

  // True, if the log should display the message like name<br>text instead of
  // name: text
  bool log_newline = false;

  // True, if the log should include RP actions like interjections, showing evidence, etc.
  bool log_ic_actions = true;

  // Margin in pixels between log entries for the IC log.
  int log_margin = 0;

  // True, if the log should have a timestamp.
  bool log_timestamp = false;

  // format string for aforementioned log timestamp
  QString log_timestamp_format;

  // True, if the log and in-character display should use custom shownames.
  bool custom_shownames = true;

  // delay before sfx plays
  QTimer *sfx_delay_timer;

  // the amount of time non-animated objection/hold it/takethat images stay
  // onscreen for in ms, and the maximum amount of time any interjections are
  // allowed to play
  const int shout_static_time = 724;
  const int shout_max_time = 1500;

  // the amount of time non-animated guilty/not guilty images stay onscreen for
  // in ms, and the maximum amount of time g/ng images are allowed to play
  const int verdict_static_time = 3000;
  const int verdict_max_time = 4000;

  // the amount of time non-animated witness testimony/cross-examination images
  // stay onscreen for in ms, and the maximum time any wt/ce image is allowed to
  // play
  const int wtce_static_time = 1500;
  const int wtce_max_time = 4000;

  // characters we consider punctuation
  const QString punctuation_chars = ".,?!:;";

  // amount by which we multiply the delay when we parse punctuation chars
  const int punctuation_modifier = 3;

  theory::IcMessagePacket m_chatmessage;
  theory::IcMessagePacket m_previous_chatmessage;

  QString additive_previous;

  // char id, muted or not
  QHash<theory::CharacterId, bool> mute_map;

  // QList<int> muted_cids;

  // state of animation, 0 = objecting, 1 = preanim, 2 = talking, 3 = idle, 4 =
  // noniterrupting preanim, 5 = (c) animation
  int anim_state = 3;

  // whether or not current color is a talking one
  bool color_is_talking = true;

  // state of text ticking, 0 = not yet ticking, 1 = ticking in progress, 2 =
  // ticking done
  int text_state = 2;

  // character id, which index of the char_list the player is
  theory::CharacterId m_character = theory::NoCharacterId;

  int objection_state = 0;
  QString objection_custom;
  struct CustomObjection
  {
    QString name;
    QString filename;
  };
  QList<CustomObjection> custom_objections_list;
  int realization_state = 0;
  int screenshake_state = 0;
  int text_color = 0;

  // How many unique user colors are possible
  static const int max_colors = 12;

  // Text Color-related optimization:
  // Current color list indexes to real color references
  QList<int> color_row_to_number;

  // List of associated RGB colors for this color index
  QList<QColor> color_rgb_list;

  // Same as above but populated from misc/default's config
  QList<QColor> default_color_rgb_list;

  // Get a color index from an arbitrary misc config
  void gen_char_rgb_list(const QString &p_misc);
  QList<QColor> char_color_rgb_list;

  // Misc we used for the last message, and the one we're using now. Used to avoid loading assets when it's not needed
  QString current_misc;
  QString last_misc;

  // List of markdown start characters, their index is tied to the color index
  QStringList color_markdown_start_list;

  // List of markdown end characters, their index is tied to the color index
  QStringList color_markdown_end_list;

  // Whether or not we're supposed to remove this char during parsing
  QList<bool> color_markdown_remove_list;

  // Whether or not this color allows us to play the talking animation
  QList<bool> color_markdown_talking_list;
  // Text Color-related optimization END

  // Current list file sorted line by line
  QStringList sound_list;

  // Current SFX the user put in for the sfx dropdown list
  QString custom_sfx;

  bool c_played = false; // whether we've played a (c)-style postanimation yet

  QString effect;

  // Music effect flags we want to send to server when we play music
  int music_flags = FADE_OUT;

  QHash<QString, int> sample_selections;

  int defense_bar_state = 0;
  int prosecution_bar_state = 0;

  int current_emote = 0;

  theory::InventoryId current_inventory = theory::NoInventoryId;

  QTimer *evidence_refresh_timer;

  // whether the ooc chat is server or master chat, true is server
  bool server_ooc = true;

  QString current_background = "default";

  // used for courtroom slide logic
  QString last_side = "";
  int last_offset = 0;
  int last_v_offset = 0;

  QString last_music_search;
  QString last_area_search;

  QBrush free_brush;
  QBrush lfp_brush;
  QBrush casing_brush;
  QBrush recess_brush;
  QBrush rp_brush;
  QBrush gaming_brush;
  QBrush building_brush;
  QBrush starting_brush;
  QBrush locked_brush;

  AOMusicPlayer *music_player;
  AOMusicPlayer *ambient_player;
  AOSfxPlayer *sfx_player;
  AOSfxPlayer *objection_player;
  AOBlipPlayer *blip_player;

  AOSfxPlayer *modcall_player;

  AOImage *ui_background;

  QWidget *ui_viewport;
  BackgroundAnimationLayer *ui_vp_background;
  SplashAnimationLayer *ui_vp_speedlines;
  CharacterAnimationLayer *ui_vp_player_char;
  CharacterAnimationLayer *ui_vp_sideplayer_char;
  CharacterAnimationLayer *ui_vp_dummy_char;
  CharacterAnimationLayer *ui_vp_sidedummy_char;
  QList<CharacterAnimationLayer *> ui_vp_char_list;
  BackgroundAnimationLayer *ui_vp_desk;
  AOEvidenceDisplay *ui_vp_evidence_display;
  AOImage *ui_vp_chatbox;
  AOChatboxLabel *ui_vp_showname;
  InterfaceAnimationLayer *ui_vp_chat_arrow;
  QTextEdit *ui_vp_message;
  SplashAnimationLayer *ui_vp_testimony;
  SplashAnimationLayer *ui_vp_wtce;
  EffectAnimationLayer *ui_vp_effect;
  SplashAnimationLayer *ui_vp_objection;

  QTextEdit *ui_ic_chatlog;

  AOTextArea *ui_debug_log;
  AOTextArea *ui_server_chatlog;

  QListWidget *ui_mute_list;
  QTreeWidget *ui_area_list;
  QTreeWidget *ui_music_list;
  PlayerListWidget *ui_player_list;

  ScrollText *ui_music_name;
  InterfaceAnimationLayer *ui_music_display;

  StickerAnimationLayer *ui_vp_sticker;

  static const int max_clocks = theory::TimerCount;
  AOClockLabel *ui_clock[max_clocks];

  AOButton *ui_pair_button;
  QListWidget *ui_pair_list;
  QSpinBox *ui_pair_offset_spinbox;
  QSpinBox *ui_pair_vert_offset_spinbox;

  QComboBox *ui_pair_order_dropdown;

  AOLineEdit *ui_ic_chat_message;
  AOLineEditFilter *ui_ic_chat_message_filter;
  QLineEdit *ui_ic_chat_name;
  QLineEdit *ui_custom_blips;

  AOLineEdit *ui_ooc_chat_message;
  QLineEdit *ui_ooc_chat_name;

  // QLineEdit *ui_area_password;
  QLineEdit *ui_music_search;

  theory::NavigableGrid *ui_emotes;
  theory::MousewheelGridNavigator *emote_navigator;
  QList<AOEmoteButton *> ui_emote_list;
  AOButton *ui_emote_left;
  AOButton *ui_emote_right;

  QMenu *emote_menu;
  AOEmotePreview *emote_preview;

  QComboBox *ui_emote_dropdown;
  QComboBox *ui_pos_dropdown;
  AOButton *ui_pos_remove;

  QComboBox *ui_iniswap_dropdown;
  AOButton *ui_iniswap_remove;

  QComboBox *ui_sfx_dropdown;
  AOButton *ui_sfx_remove;

  QComboBox *ui_effects_dropdown;

  AOImage *ui_defense_bar;
  AOImage *ui_prosecution_bar;

  QLabel *ui_music_label;
  QLabel *ui_sfx_label;
  QLabel *ui_blip_label;

  AOButton *ui_hold_it;
  AOButton *ui_objection;
  AOButton *ui_take_that;

  AOButton *ui_ooc_toggle;

  AOButton *ui_witness_testimony;
  AOButton *ui_cross_examination;
  AOButton *ui_guilty;
  AOButton *ui_not_guilty;

  AOButton *ui_change_character;
  AOButton *ui_reload_theme;
  AOButton *ui_call_mod;
  AOButton *ui_settings;
  AOButton *ui_switch_area_music;

  QCheckBox *ui_pre;
  QCheckBox *ui_flip;
  QCheckBox *ui_additive;
  QCheckBox *ui_guard;

  QCheckBox *ui_immediate;
  QCheckBox *ui_showname_enable;

  QCheckBox *ui_slide_enable;

  AOButton *ui_custom_objection;
  QMenu *custom_obj_menu;
  AOButton *ui_realization;
  AOButton *ui_screenshake;
  AOButton *ui_mute;

  AOButton *ui_defense_plus;
  AOButton *ui_defense_minus;

  AOButton *ui_prosecution_plus;
  AOButton *ui_prosecution_minus;

  QComboBox *ui_text_color;

  QSlider *ui_music_slider;
  QSlider *ui_sfx_slider;
  QSlider *ui_blip_slider;

  AOButton *ui_evidence_button;
  EvidencePanel *ui_evidence_public;
  EvidencePanel *ui_evidence_private;
  EvidencePanel *ui_evidence_current;

  AOImage *ui_char_select_background;

  // pretty list of characters
  QTreeWidget *ui_char_list;

  // abstract widget to hold char buttons
  theory::NavigableGrid *ui_char_buttons;
  theory::MousewheelGridNavigator *char_button_navigator;

  QList<AOCharButton *> ui_char_button_list;

  AOButton *ui_back_to_lobby;

  AOButton *ui_char_select_left;
  AOButton *ui_char_select_right;

  AOButton *ui_spectator;

  QLineEdit *ui_char_search;
  QCheckBox *ui_char_taken;

  void construct_char_select();
  void set_char_select();
  void set_char_buttons();
  void char_clicked(const theory::CharacterId &n_char);
  void on_char_button_context_menu_requested(const QPoint &pos);
  void filter_character_list();

  void initialize_emotes();
  void refresh_emotes();
  void set_emote_dropdown();

  void initialize_evidence();
  void refresh_evidence();
  void show_evidence(theory::EvidenceId id);
  bool inventory_editable(theory::InventoryId inventory_id) const;
  bool current_inventory_editable() const;
  theory::InventoryId personal_inventory() const;
  std::optional<PlayerInfo> evidence_inventory_owner(theory::InventoryId inventory_id) const;
  std::optional<AreaInfo> evidence_inventory_area(theory::InventoryId inventory_id) const;
  QList<InventoryInfo> listed_evidence_inventories() const;
  QString evidence_player_label(const PlayerInfo &owner) const;
  QString evidence_inventory_label(const InventoryInfo &inventory) const;
  QList<InventoryInfo> public_evidence_inventories() const;
  QString public_evidence_inventory_label(const InventoryInfo &inventory) const;
  void evidence_transfer(theory::InventoryTransferPacket::Mode mode, const QList<theory::Evidence> &list);

  void reset_ui();

  void regenerate_ic_chatlog();
public Q_SLOTS:
  void objection_done();
  void preanim_done();
  void do_screenshake();
  void do_flash();
  void do_effect(const QString &fx_path, const QString &fx_sound, const QString &p_char, const QString &p_folder);
  void play_char_sfx(const QString &sfx_name);

  void mod_called(const QString &p_ip);

  void on_reload_theme_clicked();

private Q_SLOTS:
  void refresh_clock(Timer *timer);

  void start_chat_ticking();
  void play_sfx();

  void chat_tick();

  void on_mute_list_clicked(QModelIndex p_index);
  void on_pair_list_clicked(QModelIndex p_index);

  void on_chat_return_pressed();

  void on_ooc_return_pressed();

  void on_music_search_return_pressed();
  void on_music_search_edited(const QString &p_text);
  void on_music_list_double_clicked(QTreeWidgetItem *p_item, int column);
  void on_music_list_context_menu_requested(const QPoint &pos);
  void add_favorite_song(QTreeWidgetItem *p_item);
  void remove_favorite_song(QTreeWidgetItem *p_item);
  void music_fade_out(bool toggle);
  void music_fade_in(bool toggle);
  void music_synchronize(bool toggle);
  void music_no_repeat(bool toggle);
  void music_random();
  void music_list_expand_all();
  void music_list_collapse_all();
  void music_stop(bool no_effects = false);
  void on_area_list_double_clicked(QTreeWidgetItem *p_item, int column);

  void select_emote(int p_id);

  void on_emote_left_clicked();
  void on_emote_right_clicked();
  void update_emote_arrows();

  void on_emote_dropdown_changed(int p_index);
  void on_pos_dropdown_changed(const QString &p_text);
  void on_pos_dropdown_context_menu_requested(const QPoint &pos);
  void on_pos_remove_clicked();

  void on_iniswap_dropdown_changed(int p_index);
  void set_iniswap_dropdown();
  void on_iniswap_context_menu_requested(const QPoint &pos);
  void on_iniswap_edit_requested();
  void on_iniswap_remove_clicked();

  void on_sfx_dropdown_changed(int p_index);
  void on_sfx_dropdown_custom(const QString &p_sfx);
  void set_sfx_dropdown();
  void on_sfx_context_menu_requested(const QPoint &pos);
  void on_sfx_play_clicked();
  void on_sfx_edit_requested();
  void on_sfx_remove_clicked();

  void set_effects_dropdown();
  void on_effects_context_menu_requested(const QPoint &pos);
  void on_effects_edit_requested();
  void on_character_effects_edit_requested();
  void on_effects_dropdown_changed(int p_index);
  bool effects_dropdown_find_and_set(const QString &effect);

  QString get_char_sfx();
  int get_char_sfx_delay();

  void on_hold_it_clicked();
  void on_objection_clicked();
  void on_take_that_clicked();
  void on_custom_objection_clicked();
  void show_custom_objection_menu(const QPoint &pos);

  void show_emote_menu(const QPoint &pos);

  void on_realization_clicked();
  void on_screenshake_clicked();

  void on_mute_clicked();
  void on_pair_clicked();
  void on_pair_order_dropdown_changed(int p_index);

  void on_defense_minus_clicked();
  void on_defense_plus_clicked();
  void on_prosecution_minus_clicked();
  void on_prosecution_plus_clicked();

  void on_text_color_changed(int p_color);
  void on_text_color_context_menu_requested(const QPoint &pos);
  void set_text_color_dropdown();

  void on_music_slider_moved(int p_value);
  void on_sfx_slider_moved(int p_value);
  void on_blip_slider_moved(int p_value);

  void on_log_limit_changed(int value);
  void on_pair_offset_changed(int value);
  void on_pair_vert_offset_changed(int value);

  void on_ooc_toggle_clicked();

  void on_witness_testimony_clicked();
  void on_cross_examination_clicked();
  void on_not_guilty_clicked();
  void on_guilty_clicked();

  void on_change_character_clicked();
  void on_call_mod_clicked();
  void on_settings_clicked();

  void focus_ic_input();
  void on_additive_clicked();

  void on_evidence_button_clicked();
  void on_evidence_context_menu_requested(const QPoint &pos);

  void switch_evidence_view();
  void select_evidence_inventory(theory::InventoryId inventory_id);
  void add_evidence();
  void remove_evidence(theory::EvidenceId id);
  void submit_evidence(theory::EvidenceId id, const theory::Evidence &evidence);

  void schedule_evidence_refresh();
  void refresh_evidence_inventory();

  void open_evidence_file_dialog();

  void on_char_list_double_clicked(QTreeWidgetItem *p_item, int column);
  void update_char_select_arrows();
  void on_char_search_changed();
  void on_char_taken_clicked();

  void on_spectator_clicked();

  void on_switch_area_music_clicked();

  void on_application_state_changed(Qt::ApplicationState state);

  void preview_emote(const QString &emote, CharacterAnimationLayer::EmoteType emoteType);
  void update_emote_preview();

  // After attempting to play a transition animation, clean up the viewport
  // objects for everyone else and continue the IC processing callstack
  void post_transition_cleanup();
};
} // namespace spritechat
