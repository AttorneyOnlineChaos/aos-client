#pragma once

#include "aomusictrack.h"
#include "datatypes.h"
#include "game/emote_cue.h"

#include <QColor>
#include <QHash>
#include <QList>
#include <QPoint>
#include <QSet>
#include <QString>
#include <QStringList>

namespace spritechat
{
class VPath : QString
{
  using QString::QString;

public:
  explicit VPath(const QString &str)
      : QString(str)
  {}
  inline const QString &toQString() const { return *this; }
  inline bool operator==(const VPath &str) const { return this->toQString() == str.toQString(); }
  inline VPath operator+(const VPath &str) const { return VPath(this->toQString() + str.toQString()); }
};

inline size_t qHash(const VPath &key, uint seed = QHashSeed::globalSeed())
{
  return qHash(key.toQString(), seed);
}

class AssetLookup
{
public:
  AssetLookup();

  VPath get_theme_path(const QString &p_file, QString p_theme = QString());
  VPath get_character_path(const QString &p_char, const QString &p_file);
  VPath get_misc_path(const QString &p_misc, const QString &p_file);
  VPath get_sounds_path(const QString &p_file);
  VPath get_music_path(const QString &p_song);
  VPath get_evidence_path(const QString &p_file);

  VPath get_background_path(const QString &p_file);
  VPath get_default_background_path(const QString &p_file);
  void setCurrentBackground(const QString &background);

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
  bool get_pos_is_judge(const QString &p_pos);
  int get_pos_transition_duration(const QString &old_pos, const QString &new_pos);

  QString get_case_sensitive_path(const QString &p_file);
  QString get_real_path(const VPath &vpath, const QStringList &suffixes = {""});
  QString get_sfx_suffix(const VPath &sound_to_check);
  QString get_image_suffix(const VPath &path_to_check, bool static_image = false);

  QStringList get_list_file(const VPath &path);
  QStringList get_list_file(const QString &p_file);
  QString read_file(const QString &filename);
  bool write_to_file(const QString &p_text, const QString &p_file, bool make_dir = false);
  bool append_to_file(const QString &p_text, const QString &p_file, bool make_dir = false);

  QString read_design_ini(const QString &p_identifier, const VPath &p_design_path);
  QString read_design_ini(const QString &p_identifier, const QString &p_design_path);
  QPoint get_button_spacing(const QString &p_identifier, const QString &p_file);
  pos_size_type get_element_dimensions(const QString &p_identifier, const QString &p_file, const QString &p_misc = QString());
  QString get_design_element(const QString &p_identifier, const QString &p_file, const QString &p_misc = QString());
  QColor get_color(const QString &p_identifier, const QString &p_file);
  QString get_chat_markup(const QString &p_identifier, const QString &p_file);
  QColor get_chat_color(const QString &p_identifier, const QString &p_chat);
  QString get_penalty_value(const QString &p_identifier);
  QString get_court_sfx(const QString &p_identifier, const QString &p_misc = QString());
  QString get_stylesheet(const QString &p_file);

  AOMusicTrack get_music_track(const QString &p_song);

  QString read_char_ini(const QString &p_char, const QString &p_search_line, const QString &target_tag);
  QStringList read_ini_tags(const VPath &p_file, const QString &target_tag = QString());
  QString get_char_side(const QString &p_char);
  QString get_showname(const QString &p_char, int p_emote = -1);
  QString get_category(const QString &p_char);
  QString get_chat(const QString &p_char);
  QString get_chat_font(const QString &p_char);
  int get_chat_size(const QString &p_char);
  int get_emote_number(const QString &p_char);
  QString get_emote_comment(const QString &p_char, int p_emote);
  QString get_emote(const QString &p_char, int p_emote);
  QString get_pre_emote(const QString &p_char, int p_emote);
  QString get_sfx_name(const QString &p_char, int p_emote);
  QString get_sfx_looping(const QString &p_char, int p_emote);
  QList<theory::EmoteCue> get_emote_cues(const QString &p_char, const QString &p_emote);
  int get_sfx_delay(const QString &p_char, int p_emote);
  int get_emote_mod(const QString &p_char, int p_emote);
  int get_desk_mod(const QString &p_char, int p_emote);
  QString get_blipname(const QString &p_char, int p_emote = -1);
  QString get_blips(const QString &p_blipname);
  QString get_emote_property(const QString &p_char, const QString &p_emote, const QString &p_property);

  QStringList get_effects(const QString &p_char);
  QString get_effect(const QString &effect, const QString &p_char, QString p_folder);
  QString get_effect_property(const QString &fx_name, const QString &p_char, QString p_folder, const QString &p_property);
  QString get_custom_realization(const QString &p_char);

  RESIZE_MODE get_scaling(const QString &p_scaling);
  RESIZE_MODE get_misc_scaling(const QString &p_miscname);

private:
  QString _currentBackground;

  const QString default_theme = "default";

  QHash<size_t, QString> asset_lookup_cache;
  QHash<size_t, QString> dir_listing_cache;
  QSet<size_t> dir_listing_exist_cache;
};
} // namespace spritechat
