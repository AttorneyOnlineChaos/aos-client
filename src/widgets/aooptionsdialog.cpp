#include "aooptionsdialog.h"

#include "ao_widget_lookup.h"
#include "aoapplication.h"
#include "core/logging.h"
#include "file_functions.h"
#include "network_manager.h"
#include "options.h"
#include "spritechat_defs.h"

#include <bass.h>

#include <QCollator>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QMessageBox>
#include <QResource>
#include <QUiLoader>
#include <QVBoxLayout>

spritechat::AOOptionsDialog::AOOptionsDialog(AOApplication *p_ao_app, QWidget *parent)
    : QDialog(parent)
    , ao_app(p_ao_app)
{
  setupUI();
}

void spritechat::AOOptionsDialog::populateAudioDevices()
{
  ui_audio_device_combobox->clear();
  if (needsDefaultAudioDevice())
  {
    ui_audio_device_combobox->addItem("default", "default");
  }

  BASS_DEVICEINFO info;
  for (int a = 0; BASS_GetDeviceInfo(a, &info); a++)
  {
    ui_audio_device_combobox->addItem(info.name, info.name);
  }
}

template <>
void spritechat::AOOptionsDialog::setWidgetData(QCheckBox *widget, const bool &value)
{
  widget->setChecked(value);
}

template <>
bool spritechat::AOOptionsDialog::widgetData(QCheckBox *widget) const
{
  return widget->isChecked();
}

template <>
void spritechat::AOOptionsDialog::setWidgetData(QLineEdit *widget, const QString &value)
{
  widget->setText(value);
}

template <>
QString spritechat::AOOptionsDialog::widgetData(QLineEdit *widget) const
{
  return widget->text();
}

template <>
void spritechat::AOOptionsDialog::setWidgetData(QLineEdit *widget, const uint16_t &value)
{
  widget->setText(QString::number(value));
}

template <>
uint16_t spritechat::AOOptionsDialog::widgetData(QLineEdit *widget) const
{
  return widget->text().toUShort();
}

template <>
void spritechat::AOOptionsDialog::setWidgetData(QPlainTextEdit *widget, const QStringList &value)
{
  widget->setPlainText(value.join('\n'));
}

template <>
QStringList spritechat::AOOptionsDialog::widgetData(QPlainTextEdit *widget) const
{
  return widget->toPlainText().trimmed().split('\n');
}

template <>
void spritechat::AOOptionsDialog::setWidgetData(QSpinBox *widget, const int &value)
{
  widget->setValue(value);
}

template <>
int spritechat::AOOptionsDialog::widgetData(QSpinBox *widget) const
{
  return widget->value();
}

template <>
void spritechat::AOOptionsDialog::setWidgetData(QDoubleSpinBox *widget, const double &value)
{
  widget->setValue(value);
}

template <>
double spritechat::AOOptionsDialog::widgetData(QDoubleSpinBox *widget) const
{
  return widget->value();
}

template <>
void spritechat::AOOptionsDialog::setWidgetData(QComboBox *widget, const QString &value)
{
  for (auto i = 0; i < widget->count(); i++)
  {
    if (widget->itemData(i).toString() == value)
    {
      widget->setCurrentIndex(i);
      return;
    }
  }
  zWarning(log::ui) << "value" << value << "not found for widget" << widget->objectName();
}

template <>
void spritechat::AOOptionsDialog::setWidgetData(QComboBox *widget, const RESIZE_MODE &value)
{
  widget->setCurrentIndex(value);
}

template <>
QString spritechat::AOOptionsDialog::widgetData(QComboBox *widget) const
{
  return widget->currentData().toString();
}

template <>
spritechat::RESIZE_MODE spritechat::AOOptionsDialog::widgetData(QComboBox *widget) const
{
  return RESIZE_MODE(widget->currentIndex());
}

template <>
void spritechat::AOOptionsDialog::setWidgetData(QGroupBox *widget, const bool &value)
{
  widget->setChecked(value);
}

template <>
bool spritechat::AOOptionsDialog::widgetData(QGroupBox *widget) const
{
  return widget->isChecked();
}

template <>
void spritechat::AOOptionsDialog::setWidgetData(QListWidget *widget, const QStringList &value)
{
  widget->addItems(value);
}

template <>
QStringList spritechat::AOOptionsDialog::widgetData(QListWidget *widget) const
{
  QStringList paths;
  for (auto i = 1; i < widget->count(); i++)
  {
    paths.append(widget->item(i)->text());
  }
  return paths;
}

template <typename T, typename V, typename Setter>
void spritechat::AOOptionsDialog::registerOption(const QString &widgetName, V (Options::*getter)() const, Setter setter)
{
  auto *widget = findChild<T *>(widgetName);
  if (!widget)
  {
    zWarning(log::ui) << "could not find widget" << widgetName;
    return;
  }

  OptionEntry entry;
  entry.load = [=, this] {
    setWidgetData<T, V>(widget, (Options::getInstance().*getter)());
  };
  entry.save = [=, this] {
    (Options::getInstance().*setter)(widgetData<T, V>(widget));
  };

  optionEntries.append(entry);
}

void spritechat::AOOptionsDialog::updateValues()
{
  QSet<QString> themes;
  QStringList bases = Options::getInstance().mountPaths();
  bases.push_front(get_base_path());

  for (const QString &base : bases)
  {
    QStringList l_themes = QDir(base + "/themes").entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Resorts list to match numeric sorting found in Windows.
    QCollator l_sorting;
    l_sorting.setNumericMode(true);
    std::sort(l_themes.begin(), l_themes.end(), l_sorting);

    for (const QString &l_theme : std::as_const(l_themes))
    {
      if (!themes.contains(l_theme))
      {
        ui_theme_combobox->addItem(l_theme, l_theme);
        themes.insert(l_theme);
      }
    }
  }

  QStringList l_subthemes = QDir(ao_app->get_real_path(ao_app->get_theme_path(""))).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (const QString &l_subtheme : std::as_const(l_subthemes))
  {
    if (l_subtheme.toLower() != "server" && l_subtheme.toLower() != "default" && l_subtheme.toLower() != "effects" && l_subtheme.toLower() != "misc")
    {
      ui_subtheme_combobox->addItem(l_subtheme, l_subtheme);
    }
  }

  connect(ao_app->master_gateway, &MasterGateway::privacyPolicyChanged, this, [this] {
    QString document = ao_app->master_gateway->privacyPolicy();
    if (document.isEmpty())
    {
      document = tr("Couldn't get the privacy policy.");
    }
    ui_privacy_policy->setHtml(document);
  });
  ao_app->master_gateway->requestPrivacyPolicy();

  for (const OptionEntry &entry : std::as_const(optionEntries))
  {
    entry.load();
  }
}

void spritechat::AOOptionsDialog::savePressed()
{
  bool l_reload_theme_required = (ui_theme_combobox->currentText() != Options::getInstance().theme()) || (ui_theme_scaling_factor_sb->value() != Options::getInstance().themeScalingFactor());
  for (const OptionEntry &entry : std::as_const(optionEntries))
  {
    entry.save();
  }

  if (l_reload_theme_required)
  {
    Q_EMIT reloadThemeRequest();
  }
  close();
}

void spritechat::AOOptionsDialog::discardPressed()
{
  close();
}

void spritechat::AOOptionsDialog::buttonClicked(QAbstractButton *button)
{
  if (ui_settings_buttons->buttonRole(button) == QDialogButtonBox::ResetRole)
  {
    if (QMessageBox::question(this, "", "Restore default settings?\nThis can't be undone!", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
      // Destructive operation.
      Options::getInstance().clearConfig();
      updateValues();
    }
  }
}

void spritechat::AOOptionsDialog::onReloadThemeClicked()
{
  Options::getInstance().setTheme(ui_theme_combobox->currentText());
  Options::getInstance().setSettingsSubTheme(ui_subtheme_combobox->currentText());
  Options::getInstance().setAnimatedThemeEnabled(ui_animated_theme_cb->isChecked());
  Q_EMIT reloadThemeRequest();
  delete layout();
  delete ui_settings_widget;
  optionEntries.clear();
  setupUI();
}

void spritechat::AOOptionsDialog::themeChanged(int i)
{
  ui_subtheme_combobox->clear();
  // Fill the combobox with the names of the themes.
  ui_subtheme_combobox->addItem("server", "server");
  ui_subtheme_combobox->addItem("default", "server");

  QStringList l_subthemes = QDir(ao_app->get_real_path(ao_app->get_theme_path("", ui_theme_combobox->itemText(i)))).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

  for (const QString &l_subthemes : std::as_const(l_subthemes))
  {
    if (l_subthemes.toLower() != "server" && l_subthemes.toLower() != "default" && l_subthemes.toLower() != "effects" && l_subthemes.toLower() != "misc")
    {
      ui_subtheme_combobox->addItem(l_subthemes, l_subthemes);
    }
  }

  QString l_ressource_name = Options::getInstance().theme() + ".rcc";
  QString l_resource = ao_app->get_asset("themes/" + ui_theme_combobox->currentText() + ".rcc");
  if (l_resource.isEmpty())
  {
    QResource::unregisterResource(ao_app->get_asset("themes/" + l_ressource_name));
    zDebug(log::ui) << "Unable to locate ressource file" << l_ressource_name;
    return;
  }
  QResource::registerResource(l_resource);
}

void spritechat::AOOptionsDialog::setupUI()
{
  setWindowIcon(QIcon(":/data/logo-client.png"));
  QUiLoader l_loader(this);
  QFile l_uiFile(Options::getInstance().getUIAsset("options_dialog.ui"));
  if (!l_uiFile.open(QFile::ReadOnly))
  {
    zWarning(log::ui) << "Unable to open file " << l_uiFile.fileName();
    return;
  }
  ui_settings_widget = l_loader.load(&l_uiFile, this);

  auto l_layout = new QVBoxLayout(this);
  l_layout->addWidget(ui_settings_widget);

  // General dialog element.
  AOWidgetLookup l_ui{this};

  l_ui.find(ui_settings_buttons, "settings_buttons");

  connect(ui_settings_buttons, &QDialogButtonBox::accepted, this, &AOOptionsDialog::savePressed);
  connect(ui_settings_buttons, &QDialogButtonBox::rejected, this, &AOOptionsDialog::discardPressed);
  connect(ui_settings_buttons, &QDialogButtonBox::clicked, this, &AOOptionsDialog::buttonClicked);

  // Gameplay Tab
  l_ui.find(ui_theme_combobox, "theme_combobox");
  connect(ui_theme_combobox, &QComboBox::currentIndexChanged, this, &AOOptionsDialog::themeChanged);

  registerOption<QComboBox, QString>("theme_combobox", &Options::theme, &Options::setTheme);

  l_ui.find(ui_subtheme_combobox, "subtheme_combobox");
  registerOption<QComboBox, QString>("subtheme_combobox", &Options::settingsSubTheme, &Options::setSettingsSubTheme);

  l_ui.find(ui_theme_reload_button, "theme_reload_button");
  connect(ui_theme_reload_button, &QPushButton::clicked, this, &AOOptionsDialog::onReloadThemeClicked);

  l_ui.find(ui_theme_folder_button, "theme_folder_button");
  connect(ui_theme_folder_button, &QPushButton::clicked, this, [=, this] {
    QString p_path = ao_app->get_real_path(ao_app->get_theme_path("", ui_theme_combobox->itemText(ui_theme_combobox->currentIndex())));
    if (!dir_exists(p_path))
    {
      return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(p_path));
  });

  l_ui.find(ui_theme_scaling_factor_sb, "theme_scaling_factor_sb");
  l_ui.find(ui_animated_theme_cb, "animated_theme_cb");
  l_ui.find(ui_text_crawl_spinbox, "text_crawl_spinbox");
  l_ui.find(ui_chat_ratelimit_spinbox, "chat_ratelimit_spinbox");
  l_ui.find(ui_username_textbox, "username_textbox");
  l_ui.find(ui_showname_cb, "showname_cb");
  l_ui.find(ui_default_showname_textbox, "default_showname_textbox");
  l_ui.find(ui_ms_textbox, "ms_textbox");
  l_ui.find(ui_language_combobox, "language_combobox");
  l_ui.find(ui_resize_combobox, "resize_combobox");
  l_ui.find(ui_shake_cb, "shake_cb");
  l_ui.find(ui_effects_cb, "effects_cb");
  l_ui.find(ui_framenetwork_cb, "framenetwork_cb");
  l_ui.find(ui_colorlog_cb, "colorlog_cb");
  l_ui.find(ui_stickysounds_cb, "stickysounds_cb");
  l_ui.find(ui_stickyeffects_cb, "stickyeffects_cb");
  l_ui.find(ui_stickypres_cb, "stickypres_cb");
  l_ui.find(ui_customchat_cb, "customchat_cb");
  l_ui.find(ui_sticker_cb, "sticker_cb");
  l_ui.find(ui_continuous_cb, "continuous_cb");
  l_ui.find(ui_category_stop_cb, "category_stop_cb");
  l_ui.find(ui_sfx_on_idle_cb, "sfx_on_idle_cb");
  l_ui.find(ui_evidence_double_click_cb, "evidence_double_click_cb");
  l_ui.find(ui_slides_cb, "slides_cb");
  l_ui.find(ui_restoreposition_cb, "restoreposition_cb");
  l_ui.find(ui_playerlist_format_edit, "playerlist_format_edit");

  registerOption<QDoubleSpinBox, double>("theme_scaling_factor_sb", &Options::themeScalingFactor, &Options::setThemeScalingFactor);
  registerOption<QCheckBox, bool>("animated_theme_cb", &Options::animatedThemeEnabled, &Options::setAnimatedThemeEnabled);
  registerOption<QSpinBox, int>("text_crawl_spinbox", &Options::textCrawlSpeed, &Options::setTextCrawlSpeed);
  registerOption<QSpinBox, int>("chat_ratelimit_spinbox", &Options::chatRateLimit, &Options::setChatRateLimit);
  registerOption<QLineEdit, QString>("username_textbox", &Options::username, &Options::setUsername);
  registerOption<QCheckBox, bool>("showname_cb", &Options::customShownameEnabled, &Options::setCustomShownameEnabled);
  registerOption<QLineEdit, QString>("default_showname_textbox", &Options::shownameOnJoin, &Options::setShownameOnJoin);
  registerOption<QLineEdit, QString>("ms_textbox", &Options::masterServerUrl, &Options::setMasterServerUrl);
  registerOption<QComboBox, QString>("language_combobox", &Options::language, &Options::setLanguage);

  ui_language_combobox->addItem("English", "en");
  ui_language_combobox->addItem("Deutsch", "de");
  ui_language_combobox->addItem("Español", "es");
  ui_language_combobox->addItem("Português", "pt");
  ui_language_combobox->addItem("Polski", "pl");
  ui_language_combobox->addItem("日本語", "jp");
  ui_language_combobox->addItem("Русский", "ru");

  registerOption<QComboBox, RESIZE_MODE>("resize_combobox", &Options::resizeMode, &Options::setResizeMode);
  registerOption<QCheckBox, bool>("shake_cb", &Options::shakeEnabled, &Options::setShakeEnabled);
  registerOption<QCheckBox, bool>("effects_cb", &Options::effectsEnabled, &Options::setEffectsEnabled);
  registerOption<QCheckBox, bool>("framenetwork_cb", &Options::networkedFrameSfxEnabled, &Options::setNetworkedFrameSfxEnabled);
  registerOption<QCheckBox, bool>("colorlog_cb", &Options::colorLogEnabled, &Options::setColorLogEnabled);
  registerOption<QCheckBox, bool>("stickysounds_cb", &Options::clearSoundsDropdownOnPlayEnabled, &Options::setClearSoundsDropdownOnPlayEnabled);
  registerOption<QCheckBox, bool>("stickyeffects_cb", &Options::clearEffectsDropdownOnPlayEnabled, &Options::setClearEffectsDropdownOnPlayEnabled);
  registerOption<QCheckBox, bool>("stickypres_cb", &Options::clearPreOnPlayEnabled, &Options::setClearPreOnPlayEnabled);
  registerOption<QCheckBox, bool>("customchat_cb", &Options::customChatboxEnabled, &Options::setCustomChatboxEnabled);
  registerOption<QCheckBox, bool>("sticker_cb", &Options::characterStickerEnabled, &Options::setCharacterStickerEnabled);
  registerOption<QCheckBox, bool>("continuous_cb", &Options::continuousPlaybackEnabled, &Options::setContinuousPlaybackEnabled);
  registerOption<QCheckBox, bool>("category_stop_cb", &Options::stopMusicOnCategoryEnabled, &Options::setStopMusicOnCategoryEnabled);
  registerOption<QCheckBox, bool>("sfx_on_idle_cb", &Options::playSelectedSFXOnIdle, &Options::setPlaySelectedSFXOnIdle);
  registerOption<QCheckBox, bool>("evidence_double_click_cb", &Options::evidenceDoubleClickEdit, &Options::setEvidenceDoubleClickEdit);
  registerOption<QCheckBox, bool>("slides_cb", &Options::slidesEnabled, &Options::setSlidesEnabled);
  registerOption<QCheckBox, bool>("restoreposition_cb", &Options::restoreWindowPositionEnabled, &Options::setRestoreWindowPositionEnabled);
  registerOption<QLineEdit, QString>("playerlist_format_edit", &Options::playerlistFormatString, &Options::setPlayerlistFormatString);

  // Callwords tab. This could just be a QLineEdit, but no, we decided to allow
  // people to put a billion entries in.
  l_ui.find(ui_callwords_textbox, "callwords_textbox");
  registerOption<QPlainTextEdit, QStringList>("callwords_textbox", &Options::callwords, &Options::setCallwords);
  l_ui.find(ui_callwords_sfx, "callwords_sfx");
  registerOption<QLineEdit, QString>("callwords_sfx", &Options::callwordSfx, &Options::setCallwordSfx);

  // Audio tab.
  l_ui.find(ui_audio_device_combobox, "audio_device_combobox");
  populateAudioDevices();
  registerOption<QComboBox, QString>("audio_device_combobox", &Options::audioOutputDevice, &Options::setAudioOutputDevice);

  l_ui.find(ui_suppress_audio_spinbox, "suppress_audio_spinbox");
  l_ui.find(ui_bliprate_spinbox, "bliprate_spinbox");
  l_ui.find(ui_blank_blips_cb, "blank_blips_cb");
  l_ui.find(ui_loopsfx_cb, "loopsfx_cb");
  l_ui.find(ui_objectmusic_cb, "objectmusic_cb");
  l_ui.find(ui_disablestreams_cb, "disablestreams_cb");

  registerOption<QSpinBox, int>("suppress_audio_spinbox", &Options::defaultSuppressAudio, &Options::setDefaultSupressedAudio);
  registerOption<QSpinBox, int>("bliprate_spinbox", &Options::blipRate, &Options::setBlipRate);
  registerOption<QCheckBox, bool>("blank_blips_cb", &Options::blankBlip, &Options::setBlankBlip);
  registerOption<QCheckBox, bool>("loopsfx_cb", &Options::loopingSfx, &Options::setLoopingSfx);
  registerOption<QCheckBox, bool>("objectmusic_cb", &Options::objectionStopMusic, &Options::setObjectionStopMusic);
  registerOption<QCheckBox, bool>("disablestreams_cb", &Options::streamingEnabled, &Options::setStreamingEnabled);

  // Asset tab
  l_ui.find(ui_mount_list, "mount_list");
  auto *defaultMount = new QListWidgetItem(tr("%1 (default)").arg(get_base_path()));
  defaultMount->setFlags(Qt::ItemFlag::NoItemFlags);
  ui_mount_list->addItem(defaultMount);
  registerOption<QListWidget, QStringList>("mount_list", &Options::mountPaths, &Options::setMountPaths);

  l_ui.find(ui_mount_add, "mount_add");
  connect(ui_mount_add, &QPushButton::clicked, this, [this] {
    QString path = QFileDialog::getExistingDirectory(this, tr("Select a base folder"), get_app_path(), QFileDialog::ShowDirsOnly);
    if (path.isEmpty())
    {
      return;
    }
    QDir dir(get_app_path());
    QString relative = dir.relativeFilePath(path);
    if (!relative.contains("../"))
    {
      path = relative;
    }
    QListWidgetItem *dir_item = new QListWidgetItem(path);
    ui_mount_list->addItem(dir_item);
    ui_mount_list->setCurrentItem(dir_item);

    // quick hack to update buttons
    Q_EMIT ui_mount_list->itemSelectionChanged();
  });

  l_ui.find(ui_mount_remove, "mount_remove");
  connect(ui_mount_remove, &QPushButton::clicked, this, [this] {
    auto selected = ui_mount_list->selectedItems();
    if (selected.isEmpty())
    {
      return;
    }
    delete selected[0];
    Q_EMIT ui_mount_list->itemSelectionChanged();
    asset_cache_dirty = true;
  });

  l_ui.find(ui_mount_up, "mount_up");
  connect(ui_mount_up, &QPushButton::clicked, this, [this] {
    auto selected = ui_mount_list->selectedItems();
    if (selected.isEmpty())
    {
      return;
    }
    auto *item = selected[0];
    int row = ui_mount_list->row(item);
    ui_mount_list->takeItem(row);
    int new_row = qMax(1, row - 1);
    ui_mount_list->insertItem(new_row, item);
    ui_mount_list->setCurrentRow(new_row);
    asset_cache_dirty = true;
  });

  l_ui.find(ui_mount_down, "mount_down");
  connect(ui_mount_down, &QPushButton::clicked, this, [this] {
    auto selected = ui_mount_list->selectedItems();
    if (selected.isEmpty())
    {
      return;
    }
    auto *item = selected[0];
    int row = ui_mount_list->row(item);
    ui_mount_list->takeItem(row);
    int new_row = qMin(ui_mount_list->count() + 1, row + 1);
    ui_mount_list->insertItem(new_row, item);
    ui_mount_list->setCurrentRow(new_row);
    asset_cache_dirty = true;
  });

  l_ui.find(ui_mount_clear_cache, "mount_clear_cache");
  connect(ui_mount_clear_cache, &QPushButton::clicked, this, [this] {
    asset_cache_dirty = true;
    ui_mount_clear_cache->setEnabled(false);
  });

  connect(ui_mount_list, &QListWidget::itemSelectionChanged, this, [this] {
    auto selected_items = ui_mount_list->selectedItems();
    bool row_selected = !ui_mount_list->selectedItems().isEmpty();
    ui_mount_remove->setEnabled(row_selected);
    ui_mount_up->setEnabled(row_selected);
    ui_mount_down->setEnabled(row_selected);

    if (!row_selected)
    {
      return;
    }

    int row = ui_mount_list->row(selected_items[0]);
    if (row <= 1)
    {
      ui_mount_up->setEnabled(false);
    }
    if (row >= ui_mount_list->count() - 1)
    {
      ui_mount_down->setEnabled(false);
    }
  });

  // Logging tab
  l_ui.find(ui_downwards_cb, "downwards_cb");
  l_ui.find(ui_length_spinbox, "length_spinbox");
  l_ui.find(ui_log_newline_cb, "log_newline_cb");
  l_ui.find(ui_log_margin_spinbox, "log_margin_spinbox");
  l_ui.find(ui_log_timestamp_format_lbl, "log_timestamp_format_lbl");
  l_ui.find(ui_log_timestamp_format_combobox, "log_timestamp_format_combobox");

  registerOption<QCheckBox, bool>("downwards_cb", &Options::logDirectionDownwards, &Options::setLogDirectionDownwards);
  registerOption<QSpinBox, int>("length_spinbox", &Options::maxLogSize, &Options::setMaxLogSize);
  registerOption<QCheckBox, bool>("log_newline_cb", &Options::logNewline, &Options::setLogNewline);
  registerOption<QSpinBox, int>("log_margin_spinbox", &Options::logMargin, &Options::setLogMargin);

  l_ui.find(ui_log_timestamp_cb, "log_timestamp_cb");
  registerOption<QCheckBox, bool>("log_timestamp_cb", &Options::logTimestampEnabled, &Options::setLogTimestampEnabled);
  connect(ui_log_timestamp_cb, &QCheckBox::stateChanged, this, &AOOptionsDialog::timestampCbChanged);
  ui_log_timestamp_format_lbl->setText(tr("Log timestamp format:\n") + QDateTime::currentDateTime().toString(Options::getInstance().logTimestampFormat()));

  l_ui.find(ui_log_timestamp_format_combobox, "log_timestamp_format_combobox");
  registerOption<QComboBox, QString>("log_timestamp_format_combobox", &Options::logTimestampFormat, &Options::setLogTimestampFormat);
  connect(ui_log_timestamp_format_combobox, &QComboBox::currentTextChanged, this, &AOOptionsDialog::onTimestampFormatEdited);

  QString l_current_format = Options::getInstance().logTimestampFormat();

  ui_log_timestamp_format_combobox->setCurrentText(l_current_format);

  ui_log_timestamp_format_combobox->addItem(l_current_format, l_current_format);
  ui_log_timestamp_format_combobox->addItem("h:mm:ss AP", "h:mm:ss AP");
  ui_log_timestamp_format_combobox->addItem("hh:mm:ss", "hh:mm:ss");
  ui_log_timestamp_format_combobox->addItem("h:mm AP", "h:mm AP");
  ui_log_timestamp_format_combobox->addItem("hh:mm", "hh:mm");

  if (!Options::getInstance().logTimestampEnabled())
  {
    ui_log_timestamp_format_combobox->setDisabled(true);
  }

  l_ui.find(ui_log_ic_actions_cb, "log_ic_actions_cb");
  l_ui.find(ui_log_text_cb, "log_text_cb");

  registerOption<QCheckBox, bool>("log_ic_actions_cb", &Options::logIcActions, &Options::setLogIcActions);
  registerOption<QCheckBox, bool>("log_text_cb", &Options::logToTextFileEnabled, &Options::setLogToTextFileEnabled);

  // DSGVO/Privacy tab

  l_ui.find(ui_privacy_policy, "privacy_policy");
  ui_privacy_policy->setPlainText(tr("Getting privacy policy..."));
  l_ui.find(ui_privacy_optout_cb, "privacy_optout_cb");
  registerOption<QCheckBox, bool>("privacy_optout_cb", &Options::playerCountOptout, &Options::setPlayerCountOptout);

  updateValues();
}

void spritechat::AOOptionsDialog::onTimestampFormatEdited()
{
  const QString format = ui_log_timestamp_format_combobox->currentText();
  const int index = ui_log_timestamp_format_combobox->currentIndex();

  ui_log_timestamp_format_combobox->setItemText(index, format);
  ui_log_timestamp_format_combobox->setItemData(index, format);
  ui_log_timestamp_format_lbl->setText(tr("Log timestamp format:\n") + QDateTime::currentDateTime().toString(format));
}

void spritechat::AOOptionsDialog::timestampCbChanged(int state)
{
  ui_log_timestamp_format_combobox->setDisabled(state == 0);
}

#if (defined(_WIN32) || defined(_WIN64))
bool spritechat::AOOptionsDialog::needsDefaultAudioDevice()
{
  return true;
}
#elif (defined(LINUX) || defined(__linux__))
bool spritechat::AOOptionsDialog::needsDefaultAudioDevice()
{
  return false;
}
#elif defined __APPLE__
bool spritechat::AOOptionsDialog::needsDefaultAudioDevice()
{
  return true;
}
#else
#error This operating system is not supported.
#endif
