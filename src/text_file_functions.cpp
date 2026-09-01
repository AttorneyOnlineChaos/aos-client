#include "asset_lookup.h"

#include "aoutils.h"
#include "core/json_codec.h"
#include "core/logging.h"
#include "file_functions.h"
#include "options.h"
#include "spritechat_defs.h"

#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QSettings>
#include <QStringList>
#include <QTextStream>

QStringList spritechat::AssetLookup::get_list_file(const VPath &path)
{
  return get_list_file(get_real_path(path));
}

QStringList spritechat::AssetLookup::get_list_file(const QString &p_file)
{
  QStringList return_value;

  QFile p_ini;

  p_ini.setFileName(p_file);

  if (!p_ini.open(QIODevice::ReadOnly))
  {
    return return_value;
  }

  QTextStream in(&p_ini);

  while (!in.atEnd())
  {
    QString line = in.readLine();
    return_value.append(line);
  }

  return return_value;
}

QString spritechat::AssetLookup::read_file(const QString &filename)
{
  if (filename.isEmpty())
  {
    return QString();
  }

  QFile f_log(filename);

  if (!f_log.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    zWarning(log::character) << "Couldn't open" << filename << "for reading";
    return QString();
  }

  QTextStream in(&f_log);
  QString text = in.readAll();
  f_log.close();
  return text;
}

bool spritechat::AssetLookup::write_to_file(const QString &p_text, const QString &p_file, bool make_dir)
{
  QString path = QFileInfo(p_file).path();
  if (make_dir)
  {
    // Create the dir if it doesn't exist yet
    QDir dir(path);
    if (!dir.exists())
    {
      if (!dir.mkpath("."))
      {
        return false;
      }
    }
  }

  QFile f_log(p_file);
  if (f_log.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
  {
    QTextStream out(&f_log);

    out << p_text;

    f_log.flush();
    f_log.close();
    return true;
  }
  return false;
}

bool spritechat::AssetLookup::append_to_file(const QString &p_text, const QString &p_file, bool make_dir)
{
  if (!file_exists(p_file)) // Don't create a newline if file didn't exist before now
  {
    return write_to_file(p_text, p_file, make_dir);
  }
  QString path = QFileInfo(p_file).path();
  // Create the dir if it doesn't exist yet
  if (make_dir)
  {
    QDir dir(path);
    if (!dir.exists())
    {
      if (!dir.mkpath("."))
      {
        return false;
      }
    }
  }

  QFile f_log(p_file);
  if (f_log.open(QIODevice::WriteOnly | QIODevice::Append))
  {
    QTextStream out(&f_log);

    out << "\r\n" << p_text;

    f_log.flush();
    f_log.close();
    return true;
  }
  return false;
}

QString spritechat::AssetLookup::read_design_ini(const QString &p_identifier, const VPath &p_design_path)
{
  return read_design_ini(p_identifier, get_real_path(p_design_path));
}

QString spritechat::AssetLookup::read_design_ini(const QString &p_identifier, const QString &p_design_path)
{
  QSettings settings(p_design_path, QSettings::IniFormat);
  QVariant value = settings.value(p_identifier);
  if (value.typeId() == QMetaType::QStringList)
  {
    return value.toStringList().join(",");
  }
  else if (!value.isNull())
  {
    return value.toString();
  }
  return "";
}

spritechat::RESIZE_MODE spritechat::AssetLookup::get_scaling(const QString &p_scaling)
{
  RESIZE_MODE mode = Options::getInstance().resizeMode();
  if (mode == AUTO_RESIZE_MODE)
  {
    if (p_scaling == "smooth")
    {
      mode = SMOOTH_RESIZE_MODE;
    }
    else if (p_scaling == "pixel" || p_scaling == "fast")
    {
      mode = PIXEL_RESIZE_MODE;
    }
  }

  return mode;
}

QPoint spritechat::AssetLookup::get_button_spacing(const QString &p_identifier, const QString &p_file)
{
  QString value = get_config_value(p_identifier, p_file, Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme);
  QPoint return_value;

  return_value.setX(0);
  return_value.setY(0);

  if (value == "")
  {
    return return_value;
  }

  QStringList sub_line_elements = value.split(",");

  if (sub_line_elements.size() < 2)
  {
    return return_value;
  }
  return_value.setX(sub_line_elements.at(0).toInt() * Options::getInstance().themeScalingFactor());
  return_value.setY(sub_line_elements.at(1).toInt() * Options::getInstance().themeScalingFactor());

  return return_value;
}

spritechat::pos_size_type spritechat::AssetLookup::get_element_dimensions(const QString &p_identifier, const QString &p_file, const QString &p_misc)
{
  pos_size_type return_value;
  return_value.x = 0;
  return_value.y = 0;
  return_value.width = -1;
  return_value.height = -1;
  QString f_result = get_design_element(p_identifier, p_file, p_misc);

  QStringList sub_line_elements = f_result.split(",");

  if (sub_line_elements.size() < 4)
  {
    return return_value;
  }

  int scale = Options::getInstance().themeScalingFactor();

  return_value.x = sub_line_elements.at(0).toInt() * scale;
  return_value.y = sub_line_elements.at(1).toInt() * scale;
  return_value.width = sub_line_elements.at(2).toInt() * scale;
  return_value.height = sub_line_elements.at(3).toInt() * scale;

  return return_value;
}
QString spritechat::AssetLookup::get_design_element(const QString &p_identifier, const QString &p_file, const QString &p_misc)
{
  QString value = get_config_value(p_identifier, p_file, Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme, p_misc);
  if (!value.isEmpty())
  {
    return value;
  }
  return "";
}

QColor spritechat::AssetLookup::get_color(const QString &p_identifier, const QString &p_file)
{
  QString value = get_config_value(p_identifier, p_file, Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme);
  QColor return_color(0, 0, 0);

  if (value.isEmpty())
  {
    return return_color;
  }

  QStringList color_list = value.split(",");

  if (color_list.size() < 3)
  {
    return return_color;
  }

  return_color.setRed(color_list.at(0).toInt());
  return_color.setGreen(color_list.at(1).toInt());
  return_color.setBlue(color_list.at(2).toInt());

  return return_color;
}

QString spritechat::AssetLookup::get_stylesheet(const QString &p_file)
{
  QString path = get_asset(p_file, Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme);
  QFile design_ini;
  design_ini.setFileName(path);
  if (!design_ini.open(QIODevice::ReadOnly))
  {
    return "";
  }

  QTextStream in(&design_ini);

  QString f_text;

  while (!in.atEnd())
  {
    f_text.append(in.readLine());
  }

  design_ini.close();
  return f_text;
}

QString spritechat::AssetLookup::get_chat_markup(const QString &p_identifier, const QString &p_chat)
{
  // New Chadly method
  QString value = get_config_value(p_identifier, "chat_config.ini", Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme, p_chat);
  if (!value.isEmpty())
  {
    return value.toUtf8();
  }

  // Backwards ass compatibility
  QList<VPath> backwards_paths{get_theme_path("misc/" + p_chat + "/config.ini"), VPath("misc/" + p_chat + "/config.ini"), get_theme_path("misc/default/config.ini"), VPath("misc/default/config.ini")};

  for (const VPath &p : backwards_paths)
  {
    QString value = read_design_ini(p_identifier, p);
    if (!value.isEmpty())
    {
      return value.toUtf8();
    }
  }

  return "";
}

QColor spritechat::AssetLookup::get_chat_color(const QString &p_identifier, const QString &p_chat)
{
  QColor return_color(255, 255, 255);
  QString f_result = get_chat_markup(p_identifier, p_chat);
  if (f_result == "")
  {
    return return_color;
  }

  QStringList color_list = f_result.split(",");

  if (color_list.size() < 3)
  {
    return return_color;
  }

  return_color.setRed(color_list.at(0).toInt());
  return_color.setGreen(color_list.at(1).toInt());
  return_color.setBlue(color_list.at(2).toInt());

  return return_color;
}

QString spritechat::AssetLookup::get_penalty_value(const QString &p_identifier)
{
  return get_config_value(p_identifier, "penalty/penalty.ini", Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme, "");
}

QString spritechat::AssetLookup::get_court_sfx(const QString &p_identifier, const QString &p_misc)
{
  QString value = get_config_value(p_identifier, "courtroom_sounds.ini", Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme, p_misc);
  if (!value.isEmpty())
  {
    return value.toUtf8();
  }
  return "";
}

spritechat::AOMusicTrack spritechat::AssetLookup::get_music_track(const QString &p_song)
{
  AOMusicTrack track;

  if (p_song.startsWith("http"))
  {
    track.url = QUrl(p_song);
    return track;
  }

  const QString real_path = get_real_path(get_music_path(p_song));
  if (real_path.isEmpty())
  {
    return track;
  }
  track.url = QUrl::fromLocalFile(real_path);

  const QString loop_path = real_path + ".txt";
  if (!file_exists(loop_path))
  {
    return track;
  }

  const QStringList lines = read_file(loop_path).split("\n");
  for (const QString &line : lines)
  {
    const QStringList args = line.split("=");
    if (args.size() < 2)
    {
      continue;
    }

    const QString arg = args[0].trimmed();
    const QString value = args[1].trimmed();
    if (arg == "seconds")
    {
      if (value == "true")
      {
        track.loopUnit = AOMusicTrack::LoopUnit::Second;
      }
      continue;
    }

    if (arg == "loop_start")
    {
      track.loopStart = value.toDouble();
    }
    else if (arg == "loop_length")
    {
      track.loopEnd = track.loopStart + value.toDouble();
    }
    else if (arg == "loop_end")
    {
      track.loopEnd = value.toDouble();
    }
  }

  if (track.loopStart < 0 || track.loopEnd < 0)
  {
    track.loopStart = 0;
    track.loopEnd = 0;
  }

  return track;
}

QString spritechat::AssetLookup::get_sfx_suffix(const VPath &sound_to_check)
{
  QStringList suffixes = {".opus", ".ogg", ".mp3", ".wav"};
  // Check if we were provided a direct filepath with a suffix already
  QString path = sound_to_check.toQString();
  // Loop through our suffixes
  for (const QString &suffix : suffixes)
  {
    // If our VPath ends with a valid suffix
    if (path.endsWith(suffix, Qt::CaseInsensitive))
    {
      // Return that as the path
      return get_real_path(sound_to_check);
    }
  }
  // Otherwise, ignore the provided suffix and check our own
  return get_real_path(sound_to_check, suffixes);
}

QString spritechat::AssetLookup::get_image_suffix(const VPath &path_to_check, bool static_image)
{
  QStringList suffixes{};
  if (!static_image)
  {
    suffixes.append({".webp", ".apng", ".gif"});
  }
  suffixes.append(".png");

  // Check if we were provided a direct filepath with a suffix already
  QString path = path_to_check.toQString();
  // Loop through our suffixes
  for (const QString &suffix : suffixes)
  {
    // If our VPath ends with a valid suffix
    if (path.endsWith(suffix, Qt::CaseInsensitive))
    {
      // Return that as the path
      return get_real_path(path_to_check);
    }
  }
  // Otherwise, ignore the provided suffix and check our own
  return get_real_path(path_to_check, suffixes);
}

// returns whatever is to the right of "search_line =" within target_tag and
// terminator_tag, trimmed returns the empty string if the search line couldnt
// be found
QString spritechat::AssetLookup::read_char_ini(const QString &p_char, const QString &p_search_line, const QString &target_tag)
{
  QSettings settings(get_real_path(get_character_path(p_char, "char.ini")), QSettings::IniFormat);
  settings.beginGroup(target_tag);
  QString value = settings.value(p_search_line).value<QString>();
  settings.endGroup();
  return value;
}

// returns all the values of target_tag
QStringList spritechat::AssetLookup::read_ini_tags(const VPath &p_path, const QString &target_tag)
{
  QStringList r_values;
  QSettings settings(get_real_path(p_path), QSettings::IniFormat);
  if (!target_tag.isEmpty())
  {
    settings.beginGroup(target_tag);
  }
  QStringList keys = settings.allKeys();
  for (const QString &key : std::as_const(keys))
  {
    QString value = settings.value(key).value<QString>();
    r_values << key + "=" + value;
  }
  if (!settings.group().isEmpty())
  {
    settings.endGroup();
  }
  return r_values;
}

QString spritechat::AssetLookup::get_showname(const QString &p_char, int p_emote)
{
  QString f_result = read_char_ini(p_char, "showname", "Options");
  QString f_needed = read_char_ini(p_char, "needs_showname", "Options");

  if (p_emote != -1)
  {
    int override_idx = read_char_ini(p_char, QString::number(p_emote + 1), "OptionsN").toInt();
    if (override_idx > 0)
    {
      QString override_key = "Options" + QString::number(override_idx);
      QString temp_f_result = read_char_ini(p_char, "showname", override_key);
      if (!temp_f_result.isEmpty())
      {
        f_result = temp_f_result;
      }
    }
  }

  if (f_needed.startsWith("false"))
  {
    return "";
  }
  if (f_result == "")
  {
    return p_char;
  }
  return f_result;
}

QString spritechat::AssetLookup::get_char_side(const QString &p_char)
{
  QString f_result = read_char_ini(p_char, "side", "Options");

  if (f_result == "")
  {
    return "wit";
  }
  return f_result;
}

QString spritechat::AssetLookup::get_blipname(const QString &p_char, int p_emote)
{
  QString f_result = read_char_ini(p_char, "blips", "Options");

  if (p_emote != -1)
  {
    int override_idx = read_char_ini(p_char, QString::number(p_emote + 1), "OptionsN").toInt();
    if (override_idx > 0)
    {
      QString override_key = "Options" + QString::number(override_idx);
      QString temp_f_result = read_char_ini(p_char, "blips", override_key);
      if (!temp_f_result.isEmpty())
      {
        f_result = temp_f_result;
      }
    }
  }

  if (f_result == "")
  {
    f_result = read_char_ini(p_char, "gender", "Options"); // not very PC, FanatSors
    if (f_result == "")
    {
      f_result = "male";
    }
  }
  return f_result;
}
QString spritechat::AssetLookup::get_blips(const QString &p_blipname)
{
  if (!file_exists(get_sfx_suffix(get_sounds_path(p_blipname))))
  {
    if (file_exists(get_sfx_suffix(get_sounds_path("../blips/" + p_blipname))))
    {
      return "../blips/" + p_blipname; // Return the cool kids variant
    }

    return "sfx-blip" + p_blipname; // Return legacy variant
  }
  return p_blipname;
}

QString spritechat::AssetLookup::get_emote_property(const QString &p_char, const QString &p_emote, const QString &p_property)
{
  QString f_result = read_char_ini(p_char, p_emote, p_property); // per-emote override
  if (f_result == "")
  {
    f_result = read_char_ini(p_char, p_property,
                             "Options"); // global for this character
  }
  return f_result;
}

spritechat::RESIZE_MODE spritechat::AssetLookup::get_misc_scaling(const QString &p_miscname)
{
  if (p_miscname != "")
  {
    QString misc_transform_mode = read_design_ini("scaling", get_theme_path("misc/" + p_miscname + "/config.ini"));
    if (misc_transform_mode == "")
    {
      misc_transform_mode = read_design_ini("scaling", get_misc_path(p_miscname, "config.ini"));
    }

    return get_scaling(misc_transform_mode);
  }

  return AUTO_RESIZE_MODE;
}

QString spritechat::AssetLookup::get_category(const QString &p_char)
{
  QString f_result = read_char_ini(p_char, "category", "Options");
  return f_result;
}

QString spritechat::AssetLookup::get_chat(const QString &p_char)
{
  if (p_char == "default")
  {
    return "default";
  }
  QString f_result = read_char_ini(p_char, "chat", "Options");
  return f_result;
}

QString spritechat::AssetLookup::get_chat_font(const QString &p_char)
{
  QString f_result = read_char_ini(p_char, "chat_font", "Options");

  return f_result;
}

int spritechat::AssetLookup::get_chat_size(const QString &p_char)
{
  QString f_result = read_char_ini(p_char, "chat_size", "Options");

  if (f_result == "")
  {
    return -1;
  }
  return f_result.toInt();
}

int spritechat::AssetLookup::get_emote_number(const QString &p_char)
{
  QString f_result = read_char_ini(p_char, "number", "Emotions");

  if (f_result == "")
  {
    return 0;
  }
  return f_result.toInt();
}

QString spritechat::AssetLookup::get_emote_comment(const QString &p_char, int p_emote)
{
  QString f_result = read_char_ini(p_char, QString::number(p_emote + 1), "Emotions");

  QStringList result_contents = f_result.split("#");

  if (result_contents.size() < 4)
  {
    zWarning(log::character) << "misformatted char.ini: " << p_char << ", " << p_emote;
    return "normal";
  }
  return result_contents.at(0);
}

QString spritechat::AssetLookup::get_pre_emote(const QString &p_char, int p_emote)
{
  QString f_result = read_char_ini(p_char, QString::number(p_emote + 1), "Emotions");

  QStringList result_contents = f_result.split("#");

  if (result_contents.size() < 4)
  {
    zWarning(log::character) << "misformatted char.ini: " << p_char << ", " << p_emote;
    return "";
  }
  return result_contents.at(1);
}

QString spritechat::AssetLookup::get_emote(const QString &p_char, int p_emote)
{
  QString f_result = read_char_ini(p_char, QString::number(p_emote + 1), "Emotions");

  QStringList result_contents = f_result.split("#");

  if (result_contents.size() < 4)
  {
    zWarning(log::character) << "misformatted char.ini: " << p_char << ", " << p_emote;
    return "normal";
  }
  return result_contents.at(2);
}

int spritechat::AssetLookup::get_emote_mod(const QString &p_char, int p_emote)
{
  QString f_result = read_char_ini(p_char, QString::number(p_emote + 1), "Emotions");

  QStringList result_contents = f_result.split("#");

  if (result_contents.size() < 4)
  {
    zWarning(log::character) << "misformatted char.ini: " << p_char << ", " << QString::number(p_emote);
    return 0;
  }
  return result_contents.at(3).toInt();
}

int spritechat::AssetLookup::get_desk_mod(const QString &p_char, int p_emote)
{
  QString f_result = read_char_ini(p_char, QString::number(p_emote + 1), "Emotions");

  QStringList result_contents = f_result.split("#");

  if (result_contents.size() < 5)
  {
    return -1;
  }

  QString string_result = result_contents.at(4);
  if (string_result == "")
  {
    return -1;
  }

  return string_result.toInt();
}

QString spritechat::AssetLookup::get_sfx_name(const QString &p_char, int p_emote)
{
  QString f_result = read_char_ini(p_char, QString::number(p_emote + 1), "SoundN");

  if (f_result == "")
  {
    return "1";
  }
  return f_result;
}

int spritechat::AssetLookup::get_sfx_delay(const QString &p_char, int p_emote)
{
  QString f_result = read_char_ini(p_char, QString::number(p_emote + 1), "SoundT");

  if (f_result == "")
  {
    return 0;
  }
  return f_result.toInt() * 40;
}

QString spritechat::AssetLookup::get_sfx_looping(const QString &p_char, int p_emote)
{
  QString f_result = read_char_ini(p_char, QString::number(p_emote + 1), "SoundL");

  if (f_result == "")
  {
    return "0";
  }
  else
  {
    return f_result;
  }
}

QList<theory::EmoteCue> spritechat::AssetLookup::get_emote_cues(const QString &p_char, const QString &p_emote)
{
  QList<theory::EmoteCue> cues;
  QSettings settings(get_real_path(get_character_path(p_char, "char.ini")), QSettings::IniFormat);

  auto read_section = [&](const QString &suffix, theory::EmoteCue::Type type) {
    settings.beginGroup(p_emote + suffix);
    const QStringList keys = settings.childKeys();
    for (const QString &key : keys)
    {
      bool valid_frame = false;
      const int frame = key.toInt(&valid_frame);
      if (!valid_frame || frame < 0)
      {
        continue;
      }

      theory::EmoteCue cue;
      cue.emote = p_emote;
      cue.frame = frame;
      cue.type = type;
      if (type == theory::EmoteCue::Sound)
      {
        theory::SoundEmoteCue sound;
        sound.fileName = settings.value(key).toString();
        if (sound.fileName.isEmpty())
        {
          continue;
        }
        cue.data = theory::encodeJson(sound);
      }
      cues.append(cue);
    }
    settings.endGroup();
  };

  read_section("_FrameScreenshake", theory::EmoteCue::Shake);
  read_section("_FrameRealization", theory::EmoteCue::Realization);
  read_section("_FrameSFX", theory::EmoteCue::Sound);

  return cues;
}

QStringList spritechat::AssetLookup::get_effects(const QString &p_char)
{
  const QStringList l_filepath_list{
      get_asset("effects/effects.ini", Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme, ""),
      get_asset("effects.ini", Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme, read_char_ini(p_char, "effects", "Options")),
  };

  QStringList l_effect_name_list;
  for (const QString &i_filepath : l_filepath_list)
  {
    if (!QFile::exists(i_filepath))
    {
      continue;
    }

    QSettings l_effects_ini(i_filepath, QSettings::IniFormat);
    // port legacy effects
    if (!l_effects_ini.contains("version/major") || l_effects_ini.value("version/major").toInt() < 2)
    {
      QFile effects_old(i_filepath);
      if (QFile::copy(i_filepath, i_filepath + ".old"))
      {
        migrateEffects(l_effects_ini);
      }
      else
      {
        zWarning(log::effect) << "Unable to copy effects.ini, skipping migration.";
      }
    }

    QStringList l_group_list;
    for (const QString &i_group : l_effects_ini.childGroups())
    {
      bool l_result;
      i_group.toInt(&l_result);
      if (l_result)
      {
        l_group_list.append(i_group);
      }
    }

    std::sort(l_group_list.begin(), l_group_list.end(), [](const QString &lhs, const QString &rhs) { return lhs.toInt() < rhs.toInt(); });

    for (const QString &i_group : std::as_const(l_group_list))
    {
      const QString l_key = i_group + "/name";
      if (!l_effects_ini.contains(l_key))
      {
        continue;
      }

      const QString l_effect_name = l_effects_ini.value(l_key).toString();
      if (l_effect_name.isEmpty())
      {
        continue;
      }

      l_effect_name_list.append(l_effect_name);
    }
  }
  return l_effect_name_list;
}

QString spritechat::AssetLookup::get_effect(const QString &effect, const QString &p_char, QString p_folder)
{
  if (p_folder == "")
  {
    p_folder = read_char_ini(p_char, "effects", "Options");
  }

  QStringList paths{get_image("effects/" + effect, Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme, ""), get_image(effect, Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme, p_folder)};

  for (const auto &p : paths)
  {
    if (file_exists(p))
    {
      return p;
    }
  }
  return {};
}

QString spritechat::AssetLookup::get_effect_property(const QString &fx_name, const QString &p_char, QString p_folder, const QString &p_property)
{
  if (p_folder == "")
  {
    p_folder = read_char_ini(p_char, "effects", "Options");
  }

  const auto paths = get_asset_paths("effects/effects.ini", Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme, "");
  const auto misc_paths = get_asset_paths("effects.ini", Options::getInstance().theme(), Options::getInstance().subTheme(), default_theme, p_folder);
  QString path;
  QString f_result;
  for (const VPath &p : paths + misc_paths)
  {
    path = get_real_path(p);
    if (!path.isEmpty())
    {
      QSettings settings(path, QSettings::IniFormat);
      QStringList char_effects = settings.childGroups();
      for (int i = 0; i < char_effects.size(); ++i)
      {
        QString effect = settings.value(char_effects[i] + "/name").toString();
        if (effect.toLower() == fx_name.toLower())
        {
          f_result = settings.value(char_effects[i] + "/" + p_property).toString();
          if (!f_result.isEmpty())
          {
            // Only break the loop if we get a non-empty result, continue the search otherwise
            break;
          }
        }
      }
    }
  }
  if (fx_name == "realization" && p_property == "sound")
  {
    f_result = get_custom_realization(p_char);
  }
  return f_result;
}

QString spritechat::AssetLookup::get_custom_realization(const QString &p_char)
{
  QString f_result = read_char_ini(p_char, "realization", "Options");
  if (f_result == "")
  {
    return get_court_sfx("realization");
  }
  return get_sfx_suffix(get_sounds_path(f_result));
}

bool spritechat::AssetLookup::get_pos_is_judge(const QString &p_pos)
{
  QStringList positions = read_design_ini("judges", get_background_path("design.ini")).split(",");
  if (positions.size() == 1 && positions[0] == "")
  {
    return p_pos == "jud"; // Hardcoded BS only if we have no judges= defined
  }
  return positions.contains(p_pos.trimmed());
}

int spritechat::AssetLookup::get_pos_transition_duration(const QString &old_pos, const QString &new_pos)
{
  if (old_pos.split(":").size() < 2 || new_pos.split(":").size() < 2)
  {
    return -1; // no subpositions
  }

  QString new_subpos = new_pos.split(":")[1];

  bool ok;
  int duration = read_design_ini(old_pos + "/slide_ms_" + new_subpos, get_background_path("design.ini")).toInt(&ok);
  if (ok)
  {
    return duration;
  }
  else
  {
    return -1; // invalid
  }
}
