#include "courtroom.h"

#include "core/logging.h"
#include "debug_functions.h"
#include "file_functions.h"
#include "hardware_functions.h"
#include "lobby.h"
#include "protocol/packets/session_packets.h"
#include "spritechat_log.h"

#include <QTreeWidgetItemIterator>

void spritechat::Courtroom::construct_char_select()
{
  this->setWindowFlags((this->windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

  ui_char_select_background = new AOImage(ao_app, this);
  ui_char_select_background->setObjectName("ui_char_select_background");

  ui_char_list = new QTreeWidget(ui_char_select_background);
  ui_char_list->setColumnCount(1);
  ui_char_list->setHeaderLabels({"Name"});
  ui_char_list->setHeaderHidden(true);
  ui_char_list->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui_char_list->setDropIndicatorShown(true);
  ui_char_list->setObjectName("ui_char_list");

  ui_char_buttons = new QWidget(ui_char_select_background);
  ui_char_buttons->setObjectName("ui_char_buttons");

  ui_back_to_lobby = new AOButton(ao_app, ui_char_select_background);
  ui_back_to_lobby->setObjectName("ui_back_to_lobby");

  ui_char_select_left = new AOButton(ao_app, ui_char_select_background);
  ui_char_select_left->setObjectName("ui_char_select_left");
  ui_char_select_right = new AOButton(ao_app, ui_char_select_background);
  ui_char_select_right->setObjectName("ui_char_select_right");

  ui_spectator = new AOButton(ao_app, ui_char_select_background);
  ui_spectator->setText(tr("Spectator"));
  ui_spectator->setObjectName("ui_spectator");

  ui_char_search = new QLineEdit(ui_char_select_background);
  ui_char_search->setPlaceholderText(tr("Search"));
  ui_char_search->setObjectName("ui_char_search");

  ui_char_taken = new QCheckBox(ui_char_select_background);
  ui_char_taken->setText(tr("Taken"));
  ui_char_taken->setObjectName("ui_char_taken");

  connect(ui_char_list, &QTreeWidget::itemDoubleClicked, this, &Courtroom::on_char_list_double_clicked);

  connect(ui_back_to_lobby, &AOButton::clicked, this, &Courtroom::close);

  connect(ui_char_select_left, &AOButton::clicked, this, &Courtroom::on_char_select_left_clicked);
  connect(ui_char_select_right, &AOButton::clicked, this, &Courtroom::on_char_select_right_clicked);

  connect(ui_spectator, &AOButton::clicked, this, &Courtroom::on_spectator_clicked);

  connect(ui_char_search, &QLineEdit::textEdited, this, &Courtroom::on_char_search_changed);
  connect(ui_char_taken, &QCheckBox::stateChanged, this, &Courtroom::on_char_taken_clicked);
}

void spritechat::Courtroom::set_char_select()
{
  QString filename = "courtroom_design.ini";

  pos_size_type f_charselect = ao_app->get_element_dimensions("char_select", filename);

  if (f_charselect.width < 0 || f_charselect.height < 0)
  {
    zWarning(log::ui) << "did not find char_select width or height in "
                         "courtroom_design.ini!";
    this->setFixedSize(714, 668);
  }
  else
  {
    this->setFixedSize(f_charselect.width, f_charselect.height);
  }
  ui_char_select_background->resize(f_charselect.width, f_charselect.height);
  ui_char_select_background->setImage("charselect_background");

  ui_char_search->setFocus();
  set_size_and_pos(ui_char_search, "char_search");
  set_size_and_pos(ui_char_list, "char_list");
  set_size_and_pos(ui_char_taken, "char_taken");
  set_size_and_pos(ui_char_buttons, "char_buttons");

  // Silence emission. This causes the signal to be emitted TWICE during server join!
  // Fuck this. Performance Sandwich.
  ui_char_taken->blockSignals(true);
  ui_char_taken->setChecked(true);
  ui_char_taken->blockSignals(false);

  truncate_label_text(ui_char_taken, "char_taken");

  ui_char_select_background->show();

  filter_character_list();

  ui_char_search->setFocus();
}

void spritechat::Courtroom::set_char_select_page()
{
  ui_char_select_left->hide();
  ui_char_select_right->hide();

  for (AOCharButton *i_button : std::as_const(ui_char_button_list))
  {
    i_button->hide();
    i_button->move(0, 0);
  }

  int total_pages = ui_char_button_list_filtered.size() / max_chars_on_page;
  int chars_on_page = 0;

  if (ui_char_button_list_filtered.size() % max_chars_on_page != 0)
  {
    ++total_pages;
    // i. e. not on the last page
    if (total_pages > current_char_page + 1)
    {
      chars_on_page = max_chars_on_page;
    }
    else
    {
      chars_on_page = ui_char_button_list_filtered.size() % max_chars_on_page;
    }
  }
  else
  {
    chars_on_page = max_chars_on_page;
  }

  if (total_pages > current_char_page + 1)
  {
    ui_char_select_right->show();
  }

  if (current_char_page > 0)
  {
    ui_char_select_left->show();
  }

  QPoint f_spacing = ao_app->get_button_spacing("char_button_spacing", "courtroom_design.ini");

  int s_button_size = button_width * Options::getInstance().themeScalingFactor();

  char_columns = ((ui_char_buttons->width() - s_button_size) / (f_spacing.x() + s_button_size)) + 1;
  char_rows = ((ui_char_buttons->height() - s_button_size) / (f_spacing.y() + s_button_size)) + 1;

  max_chars_on_page = char_columns * char_rows;

  put_button_in_place(current_char_page * max_chars_on_page, chars_on_page);
}

void spritechat::Courtroom::on_char_list_double_clicked(QTreeWidgetItem *p_item, int column)
{
  Q_UNUSED(column);
  theory::CharacterId cid{p_item->data(0, Qt::UserRole).toString()};
  if (cid == theory::NoCharacterId && !p_item->isExpanded())
  {
    p_item->setExpanded(true);
    return;
  }
  else if (cid == theory::NoCharacterId)
  {
    p_item->setExpanded(false);
    return;
  }
  char_clicked(cid);
}

void spritechat::Courtroom::char_clicked(const theory::CharacterId &n_char)
{
  if (n_char != theory::NoCharacterId)
  {
    QString char_name = n_char.toString();
    QString char_ini_path = ao_app->get_real_path(ao_app->get_character_path(char_name, "char.ini"));

    if (!file_exists(char_ini_path))
    {
      call_warning(tr("Could not find character (char.ini) for %1").arg(char_name));
      return;
    }

    zDebug(log::character) << "Found char.ini for" << char_name << "at" << char_ini_path;
  }

  if (n_char != m_character || n_char == theory::NoCharacterId)
  {
    theory::ChangeCharacterPacket changePacket;
    changePacket.character = n_char;
    transport.shipPacket(changePacket);
  }
  if (n_char == m_character || n_char == theory::NoCharacterId)
  {
    update_character(n_char);
    enter_courtroom();
    set_courtroom_size();
  }
}

void spritechat::Courtroom::on_char_button_context_menu_requested(const QPoint &pos)
{
  AOCharButton *button = qobject_cast<AOCharButton *>(sender());
  theory::CharacterId n_char = button->character();
  if (n_char == theory::NoCharacterId)
  {
    return;
  }

  QString char_name = n_char.toString();
  QString char_ini_path = ao_app->get_real_path(ao_app->get_character_path(char_name, "char.ini"));

  if (!file_exists(char_ini_path))
  {
    call_warning(tr("Could not find character (char.ini) for %1").arg(char_name));
    return;
  }

  QMenu *menu = new QMenu(this);
  menu->addAction(QString("Edit " + char_name + "/char.ini"), this, [=, this] { QDesktopServices::openUrl(QUrl::fromLocalFile(char_ini_path)); });
  menu->addSeparator();
  menu->addAction(QString("Open character folder " + char_name), this, [=, this] {
    QString p_path = ao_app->get_real_path(VPath("characters/" + char_name + "/"));
    if (!dir_exists(p_path))
    {
      return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(p_path));
  });
  menu->popup(button->mapToGlobal(pos));
}

void spritechat::Courtroom::put_button_in_place(int starting, int chars_on_this_page)
{
  if (ui_char_button_list_filtered.size() == 0)
  {
    return;
  }

  QPoint f_spacing = ao_app->get_button_spacing("char_button_spacing", "courtroom_design.ini");

  int x_mod_count = 0;
  int y_mod_count = 0;

  int startout = starting;
  int size = button_width * Options::getInstance().themeScalingFactor();
  for (int n = starting; n < startout + chars_on_this_page; ++n)
  {
    int x_pos = (size + f_spacing.x()) * x_mod_count;
    int y_pos = (size + f_spacing.y()) * y_mod_count;

    ui_char_button_list_filtered.at(n)->move(x_pos, y_pos);
    ui_char_button_list_filtered.at(n)->show();

    ++x_mod_count;

    if (x_mod_count == char_columns)
    {
      ++y_mod_count;
      x_mod_count = 0;
    }
  }
}

void spritechat::Courtroom::character_loading_finished()
{
  // Zeroeth, we'll clear any leftover characters from previous server visits.
  if (ui_char_button_list.size() > 0)
  {
    for (AOCharButton *item : std::as_const(ui_char_button_list))
    {
      delete item;
    }
    ui_char_button_list.clear();
    ui_char_list->clear();
  }

  // First, we'll make all the character buttons in the very beginning.
  // We also hide them all, so they can't be accidentally clicked.
  // Later on, we'll be revealing buttons as we need them.
  for (const theory::CharacterId &character : std::as_const(char_list))
  {
    AOCharButton *char_button = new AOCharButton(ao_app, ui_char_buttons);
    char_button->setContextMenuPolicy(Qt::CustomContextMenu);
    char_button->hide();
    char_button->setCharacter(character);
    char_button->setTaken(taken_chars.contains(character));
    char_button->setToolTip(character.toString());
    ui_char_button_list.append(char_button);
    QString char_category = ao_app->get_category(character.toString());
    QList<QTreeWidgetItem *> matching_list = ui_char_list->findItems(char_category, Qt::MatchFixedString, 0);
    // create the character tree item
    QTreeWidgetItem *treeItem = new QTreeWidgetItem();
    treeItem->setText(0, character.toString());
    treeItem->setIcon(0, QIcon(ao_app->get_image_suffix(ao_app->get_character_path(character.toString(), "char_icon"), true)));
    treeItem->setData(0, Qt::UserRole, character.toString());
    // category logic
    QTreeWidgetItem *category;
    if (char_category == "") // no category
    {
      ui_char_list->addTopLevelItem(treeItem);
    }
    else if (!matching_list.isEmpty())
    { // our category already exists
      category = matching_list[0];
      category->addChild(treeItem);
    }
    else
    { // we need to make a new category
      category = new QTreeWidgetItem();
      category->setText(0, char_category);
      category->setData(0, Qt::UserRole, theory::NoCharacterId.toString());
      category->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
      ui_char_list->insertTopLevelItem(0, category);
      category->addChild(treeItem);
    }

    connect(char_button, &AOCharButton::clicked, this, [this, character]() { this->char_clicked(character); });
    connect(char_button, &AOCharButton::customContextMenuRequested, this, &Courtroom::on_char_button_context_menu_requested);
  }
  ui_char_list->sortItems(0, Qt::AscendingOrder);
  ui_char_list->expandAll();

  filter_character_list();
}

void spritechat::Courtroom::filter_character_list()
{
  ui_char_button_list_filtered.clear();
  for (AOCharButton *current_char : std::as_const(ui_char_button_list))
  {
    const theory::CharacterId character = current_char->character();
    QTreeWidgetItem *current_char_item = nullptr;
    for (QTreeWidgetItemIterator it(ui_char_list); *it; ++it)
    {
      if (theory::CharacterId{(*it)->data(0, Qt::UserRole).toString()} == character)
      {
        current_char_item = *it;
        break;
      }
    }

    if (!current_char_item)
    {
      continue;
    }

    if (!ui_char_taken->isChecked() && taken_chars.contains(character))
    {
      current_char_item->setHidden(true);
      continue;
    }

    if (!character->contains(ui_char_search->text(), Qt::CaseInsensitive) && !ao_app->get_category(character.toString()).contains(ui_char_search->text(), Qt::CaseInsensitive))
    {
      current_char_item->setHidden(true);
      continue;
    }

    // We only really need to update the fact that a character is taken
    // for the buttons that actually appear.
    // You'd also update the passwordedness and etc. here later.
    current_char_item->setHidden(false);
    current_char->setTaken(taken_chars.contains(character));
    current_char_item->setText(0, character.toString());
    // reset disabled
    current_char_item->setDisabled(false);
    if (taken_chars.contains(character)) // woops, we are taken
    {
      current_char_item->setDisabled(true);
    }

    ui_char_button_list_filtered.append(current_char);
  }

  current_char_page = 0;
  set_char_select_page();
}

void spritechat::Courtroom::on_char_search_changed()
{
  filter_character_list();
}

void spritechat::Courtroom::on_char_taken_clicked()
{
  filter_character_list();
}
