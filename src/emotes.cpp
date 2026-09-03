#include "courtroom.h"

#include "aoemotebutton.h"
#include "options.h"

void spritechat::Courtroom::initialize_emotes()
{
  ui_emotes = new theory::NavigableGrid(this);
  ui_emotes->setObjectName("ui_emotes");
  emote_navigator = new theory::MousewheelGridNavigator(ui_emotes);

  ui_emote_left = new AOButton(ao_app, this);
  ui_emote_left->setObjectName("ui_emote_left");
  ui_emote_right = new AOButton(ao_app, this);
  ui_emote_right->setObjectName("ui_emote_right");

  ui_emote_dropdown = new QComboBox(this);
  ui_emote_dropdown->setContextMenuPolicy(Qt::CustomContextMenu);
  ui_emote_dropdown->setObjectName("ui_emote_dropdown");

  emote_menu = new QMenu(this);
  emote_menu->setObjectName("ui_emote_menu");

  emote_preview = new AOEmotePreview(ao_app, this);
  emote_preview->setObjectName("ui_emote_preview");
  emote_preview->resize(256, 192);

  connect(ui_emote_left, &AOButton::clicked, this, &Courtroom::on_emote_left_clicked);
  connect(ui_emote_right, &AOButton::clicked, this, &Courtroom::on_emote_right_clicked);
  connect(ui_emotes, &theory::NavigableGrid::layoutUpdated, this, &Courtroom::update_emote_arrows);
  connect(ui_emotes, &theory::NavigableGrid::currentPageChanged, this, &Courtroom::update_emote_arrows);

  connect(ui_emote_dropdown, &QComboBox::activated, this, &Courtroom::on_emote_dropdown_changed);
  connect(ui_emote_dropdown, &AOEmoteButton::customContextMenuRequested, this, &Courtroom::show_emote_menu);

  connect(ui_pre, &QCheckBox::stateChanged, this, &Courtroom::update_emote_preview);
  connect(ui_flip, &AOButton::clicked, this, &Courtroom::update_emote_preview);
  connect(ui_pair_offset_spinbox, &QSpinBox::valueChanged, this, &Courtroom::update_emote_preview);
  connect(ui_pair_vert_offset_spinbox, &QSpinBox::valueChanged, this, &Courtroom::update_emote_preview);
}

void spritechat::Courtroom::refresh_emotes()
{
  // Should properly refresh the emote list
  qDeleteAll(ui_emote_list.begin(), ui_emote_list.end());
  ui_emote_list.clear();

  set_size_and_pos(ui_emotes, "emotes");

  set_size_and_pos(ui_emote_left, "emote_left");
  ui_emote_left->setImage("arrow_left");

  set_size_and_pos(ui_emote_right, "emote_right");
  ui_emote_right->setImage("arrow_right");

  QPoint f_spacing = ao_app->get_button_spacing("emote_button_spacing", "courtroom_design.ini");
  QPoint p_point = ao_app->get_button_spacing("emote_button_size", "courtroom_design.ini");
  ui_emotes->setHorizontalSpacing(f_spacing.x());
  ui_emotes->setVerticalSpacing(f_spacing.y());

  QString selected_image = ao_app->get_image_suffix(ao_app->get_theme_path("emote_selected", ""), true);
  QString character = m_character.toString();
  int total_emotes = ao_app->get_emote_number(character);

  QList<QWidget *> buttons;
  for (int i = 0; i < total_emotes; ++i)
  {
    AOEmoteButton *f_emote = new AOEmoteButton(i, p_point.x(), p_point.y(), ao_app, ui_emotes);
    f_emote->setSelectedImage(selected_image);
    f_emote->setImage(character, i == current_emote);
    f_emote->setToolTip(QString::number(i + 1) + ": " + ao_app->get_emote_comment(character, i));
    ui_emote_list.append(f_emote);
    buttons.append(f_emote);

    f_emote->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(f_emote, &AOEmoteButton::emoteClicked, this, &Courtroom::select_emote);
    connect(f_emote, &AOEmoteButton::customContextMenuRequested, this, &Courtroom::show_emote_menu);
  }
  ui_emotes->setWidgets(buttons);
}

void spritechat::Courtroom::set_emote_dropdown()
{
  ui_emote_dropdown->clear();

  int total_emotes = ao_app->get_emote_number(m_character.toString());

  for (int n = 0; n < total_emotes; ++n)
  {
    ui_emote_dropdown->addItem(QString::number(n + 1) + ": " + ao_app->get_emote_comment(m_character.toString(), n));
    QString icon_path = ao_app->get_image_suffix(ao_app->get_character_path(m_character.toString(), "emotions/button" + QString::number(n + 1) + "_off"));
    ui_emote_dropdown->setItemIcon(n, QIcon(icon_path));
  }
  if (current_emote > -1 && current_emote < ui_emote_dropdown->count())
  {
    ui_emote_dropdown->setCurrentIndex(current_emote);
  }
}

void spritechat::Courtroom::select_emote(int p_id)
{
  if (current_emote < ui_emote_list.size())
  {
    ui_emote_list.at(current_emote)->setImage(m_character.toString(), false);
  }

  int old_emote = current_emote;

  current_emote = p_id;
  ui_emote_list.at(current_emote)->setImage(m_character.toString(), true);

  int emote_mod = ao_app->get_emote_mod(m_character.toString(), current_emote);

  if (old_emote == current_emote)
  {
    ui_pre->setChecked(!ui_pre->isChecked());
  }
  else if (!Options::getInstance().clearPreOnPlayEnabled())
  {
    if (emote_mod == PREANIM || emote_mod == PREANIM_ZOOM)
    {
      ui_pre->setChecked(true);
    }
    else
    {
      ui_pre->setChecked(false);
    }
  }

  ui_emote_dropdown->setCurrentIndex(current_emote);
  update_emote_preview();
  ui_ic_chat_message->setFocus();
}

void spritechat::Courtroom::update_emote_preview()
{
  if (!emote_preview->isVisible())
  {
    return;
  }

  QString pre = ao_app->get_pre_emote(m_character.toString(), current_emote);
  if (ui_pre->isChecked() && !pre.isEmpty() && pre != "-")
  {
    preview_emote(pre, CharacterAnimationLayer::PreEmote);
  }
  else
  {
    preview_emote(ao_app->get_emote(m_character.toString(), current_emote), CharacterAnimationLayer::IdleEmote);
  }
}

void spritechat::Courtroom::show_emote_menu(const QPoint &pos)
{
  QWidget *button = qobject_cast<QWidget *>(sender());
  int id = current_emote;
  if (qobject_cast<AOEmoteButton *>(button))
  {
    AOEmoteButton *emote_button = qobject_cast<AOEmoteButton *>(sender());
    id = emote_button->id();
  }
  emote_menu->clear();
  emote_menu->setDefaultAction(emote_menu->addAction("Preview Selected", this, [this] {
    emote_preview->show();
    emote_preview->raise();
    emote_preview->updateViewportGeometry();
    update_emote_preview();
  }));
  QString f_pre = ao_app->get_pre_emote(m_character.toString(), id);
  if (!f_pre.isEmpty() && f_pre != "-")
  {
    emote_menu->addAction("Preview preanimation: " + f_pre, this, [this, f_pre] { preview_emote(f_pre, CharacterAnimationLayer::PreEmote); });
  }

  QString f_emote = ao_app->get_emote(m_character.toString(), id);
  if (!f_emote.isEmpty())
  {
    emote_menu->addAction("Preview idle: " + f_emote, this, [this, f_emote] { preview_emote(f_emote, CharacterAnimationLayer::IdleEmote); });
    emote_menu->addAction("Preview talk: " + f_emote, this, [this, f_emote] { preview_emote(f_emote, CharacterAnimationLayer::TalkEmote); });
    QStringList c_paths = {ao_app->get_image_suffix(ao_app->get_character_path(m_character.toString(), "(c)" + f_emote)), ao_app->get_image_suffix(ao_app->get_character_path(m_character.toString(), "(c)/" + f_emote))};
    // if there is a (c) animation
    if (file_exists(ao_app->find_image(c_paths)))
    {
      emote_menu->addAction("Preview postanimation: " + f_emote, this, [this, f_emote] { preview_emote(f_emote, CharacterAnimationLayer::PostEmote); });
    }
  }
  emote_menu->popup(button->mapToGlobal(pos));
}

void spritechat::Courtroom::preview_emote(const QString &f_emote, CharacterAnimationLayer::EmoteType emoteType)
{
  emote_preview->show();
  emote_preview->raise();
  emote_preview->updateViewportGeometry();
  emote_preview->display(m_character.toString(), f_emote, emoteType, ui_flip->isChecked(), ui_pair_offset_spinbox->value(), -ui_pair_vert_offset_spinbox->value());
}

void spritechat::Courtroom::on_emote_left_clicked()
{
  ui_emotes->previousPage();
  ui_ic_chat_message->setFocus();
}

void spritechat::Courtroom::on_emote_right_clicked()
{
  ui_emotes->nextPage();
  ui_ic_chat_message->setFocus();
}

void spritechat::Courtroom::update_emote_arrows()
{
  ui_emote_left->setVisible(ui_emotes->currentPage() > 0);
  ui_emote_right->setVisible(ui_emotes->currentPage() + 1 < ui_emotes->pageCount());
}

void spritechat::Courtroom::on_emote_dropdown_changed(int p_index)
{
  select_emote(p_index);
}
