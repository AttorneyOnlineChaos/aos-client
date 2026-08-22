#include "lobby.h"

#include "ao_widget_lookup.h"
#include "aoutils.h"
#include "core/logging.h"
#include "file_functions.h"
#include "options.h"
#include "spritechat_info.h"
#include "spritechat_log.h"
#include "widgets/aooptionsdialog.h"
#include "widgets/direct_connect_dialog.h"
#include "widgets/server_editor_dialog.h"

#include <QImageReader>
#include <QMessageBox>
#include <QUiLoader>
#include <QVersionNumber>

#include <utility>

spritechat::Lobby::Lobby(AOApplication *p_ao_app, NetworkManager &network, MasterGateway &master)
    : QMainWindow{}
    , ao_app{p_ao_app}
    , net_manager{network}
    , master_gateway{master}
{
  server_info_gateway = new ServerInfoGateway(this);
  connect(server_info_gateway, &ServerInfoGateway::infoSettled, this, &Lobby::on_server_info_settled);

  connect(&net_manager, &NetworkManager::statusChanged, this, &Lobby::update_connect_button);

  connect(&master_gateway, &MasterGateway::serverListChanged, this, &Lobby::list_servers);
  connect(&master_gateway, &MasterGateway::messageOfTheDayChanged, this, &Lobby::show_message_of_the_day);
  connect(&master_gateway, &MasterGateway::versionChanged, this, &Lobby::warn_about_outdated_client);

  reloadUi();
  setObjectName("lobby");
}

void spritechat::Lobby::on_tab_changed(int index)
{
  switch (index)
  {
  case SERVER:
    current_page = SERVER;
    ui_add_to_favorite_button->setVisible(true);
    ui_remove_from_favorites_button->setVisible(false);
    ui_add_server_button->setVisible(false);
    ui_edit_favorite_button->setVisible(false);
    ui_direct_connect_button->setVisible(true);
    reset_selection();
    break;
  case FAVORITES:
    current_page = FAVORITES;
    ui_add_to_favorite_button->setVisible(false);
    ui_remove_from_favorites_button->setVisible(true);
    ui_add_server_button->setVisible(true);
    ui_edit_favorite_button->setVisible(true);
    ui_direct_connect_button->setVisible(false);
    reset_selection();
    break;
  case DEMOS:
    current_page = DEMOS;
    ui_add_to_favorite_button->setVisible(false);
    ui_add_server_button->setVisible(false);
    ui_remove_from_favorites_button->setVisible(false);
    ui_edit_favorite_button->setVisible(false);
    ui_direct_connect_button->setVisible(false);
    reset_selection();
    QMessageBox::information(this, tr("Demos"), tr("Demo recording and playback has been removed for now. The functionality will come back later."));
    break;
  default:
    break;
  }
}

int spritechat::Lobby::get_selected_server() const
{
  switch (ui_connections_tabview->currentIndex())
  {
  case SERVER:
    if (auto item = ui_serverlist_tree->currentItem())
    {
      return item->text(0).toInt();
    }
    break;
  case FAVORITES:
    if (auto item = ui_favorites_tree->currentItem())
    {
      return item->text(0).toInt();
    }
    break;
  default:
    break;
  }
  return -1;
}

std::optional<spritechat::ServerBookmark> spritechat::Lobby::current_server() const
{
  QList<ServerBookmark> servers;
  switch (ui_connections_tabview->currentIndex())
  {
  default:
    return std::nullopt;
  case SERVER:
    servers = master_gateway.serverList();
    break;
  case FAVORITES:
    servers = Options::getInstance().favorites();
    break;
  }

  const int index = get_selected_server();
  if (index < 0 || index >= servers.size())
  {
    return std::nullopt;
  }

  return servers.at(index);
}

void spritechat::Lobby::reset_selection()
{
  last_index = -1;
  set_server_status(ServerStatus::Offline);
  ui_server_description_text->clear();

  ui_edit_favorite_button->setEnabled(false);
  ui_remove_from_favorites_button->setEnabled(false);
  ui_connect_button->setEnabled(false);
}

void spritechat::Lobby::loadUI()
{
  setWindowIcon(QIcon(":/data/logo-client.png"));
  setWindowFlags((windowFlags() | Qt::CustomizeWindowHint));

  QUiLoader l_loader(this);
  QFile l_uiFile(Options::getInstance().getUIAsset(DEFAULT_UI));
  if (!l_uiFile.open(QFile::ReadOnly))
  {
    zCritical(log::ui) << "Unable to open file " << l_uiFile.fileName();
    return;
  }

  l_loader.load(&l_uiFile, this);

  AOWidgetLookup l_ui{this};

  l_ui.find(ui_game_version_lbl, "game_version_lbl");
  ui_game_version_lbl->setText(tr("Version: %1").arg(softwareVersion().toString()));

  l_ui.find(ui_settings_button, "settings_button");
  connect(ui_settings_button, &QPushButton::clicked, this, &Lobby::onSettingsRequested);

  l_ui.find(ui_about_button, "about_button");
  connect(ui_about_button, &QPushButton::clicked, this, &Lobby::on_about_clicked);

  // Serverlist elements
  l_ui.find(ui_connections_tabview, "connections_tabview");
  ui_connections_tabview->tabBar()->setExpanding(true);
  connect(ui_connections_tabview, &QTabWidget::currentChanged, this, &Lobby::on_tab_changed);

  l_ui.find(ui_serverlist_tree, "serverlist_tree");
  l_ui.find(ui_serverlist_search, "serverlist_search");
  connect(ui_serverlist_tree, &QTreeWidget::itemClicked, this, &Lobby::on_server_list_clicked);
  connect(ui_serverlist_tree, &QTreeWidget::itemDoubleClicked, this, &Lobby::on_list_doubleclicked);
  connect(ui_serverlist_search, &QLineEdit::textChanged, this, &Lobby::on_server_search_edited);

  l_ui.find(ui_favorites_tree, "favorites_tree");
  connect(ui_favorites_tree, &QTreeWidget::itemClicked, this, &Lobby::on_favorite_tree_clicked);
  connect(ui_favorites_tree, &QTreeWidget::itemDoubleClicked, this, &Lobby::on_list_doubleclicked);

  l_ui.find(ui_refresh_button, "refresh_button");
  connect(ui_refresh_button, &QPushButton::released, this, &Lobby::on_refresh_released);

  l_ui.find(ui_direct_connect_button, "direct_connect_button");
  connect(ui_direct_connect_button, &QPushButton::released, this, &Lobby::on_direct_connect_released);

  l_ui.find(ui_add_to_favorite_button, "add_to_favorite_button");
  connect(ui_add_to_favorite_button, &QPushButton::released, this, &Lobby::on_add_to_fav_released);

  l_ui.find(ui_add_server_button, "add_server_button");
  ui_add_server_button->setVisible(false);
  connect(ui_add_server_button, &QPushButton::released, this, &Lobby::on_add_server_to_fave_released);

  l_ui.find(ui_edit_favorite_button, "edit_favorite_button");
  ui_edit_favorite_button->setVisible(false);
  connect(ui_edit_favorite_button, &QPushButton::released, this, &Lobby::on_edit_favorite_released);

  l_ui.find(ui_remove_from_favorites_button, "remove_from_favorites_button");
  ui_remove_from_favorites_button->setVisible(false);
  connect(ui_remove_from_favorites_button, &QPushButton::released, this, &Lobby::on_remove_from_fav_released);

  l_ui.find(ui_server_player_count_lbl, "server_player_count_lbl");
  l_ui.find(ui_server_description_text, "server_description_text");
  l_ui.find(ui_connect_button, "connect_button");
  connect(ui_connect_button, &QPushButton::released, this, &Lobby::on_connect_released);

  l_ui.find(ui_motd_text, "motd_text");
  l_ui.find(ui_game_changelog_text, "game_changelog_text");
  if (ui_game_changelog_text != nullptr)
  {
    QString l_changelog_text = "No changelog found.";
    QFile l_changelog(get_base_path() + "changelog.md");
    if (!l_changelog.open(QFile::ReadOnly))
    {
      zDebug(log::ui) << "Unable to locate changelog file. Does it even exist?";

      ui_game_changelog_text->setMarkdown(l_changelog_text);
      return;
    }

    ui_game_changelog_text->setMarkdown(l_changelog.readAll());
    l_changelog.close();

    QTabWidget *l_tabbar = findChild<QTabWidget *>("motd_changelog_tab");
    if (l_tabbar != nullptr)
    {
      l_tabbar->tabBar()->setExpanding(true);
    }
  }
}

void spritechat::Lobby::reloadUi()
{
  loadUI();
  list_servers();
  list_favorites();
  get_motd();
  check_for_updates();
  reset_selection();
}

void spritechat::Lobby::on_refresh_released()
{
  master_gateway.requestServerList();
  get_motd();
  list_favorites();
}

void spritechat::Lobby::on_direct_connect_released()
{
  DirectConnectDialog connect_dialog;
  connect(&connect_dialog, &DirectConnectDialog::connection_requested, this, &Lobby::connection_requested);
  connect_dialog.exec();
}

void spritechat::Lobby::on_add_to_fav_released()
{
  int selection = get_selected_server();
  if (selection > -1)
  {
    Options::getInstance().addFavorite(master_gateway.serverList().at(selection));
    list_favorites();
  }
}

void spritechat::Lobby::on_add_server_to_fave_released()
{
  ServerEditorDialog dialog;
  if (dialog.exec())
  {
    Options::getInstance().addFavorite(dialog.currentServerBookmark());
    list_favorites();
    reset_selection();
  }
}

void spritechat::Lobby::on_edit_favorite_released()
{
  const int index = get_selected_server();
  ServerEditorDialog dialog(Options::getInstance().favorites().at(index));
  if (dialog.exec())
  {
    Options::getInstance().updateFavorite(dialog.currentServerBookmark(), index);
    list_favorites();
    reset_selection();
  }
}

void spritechat::Lobby::on_remove_from_fav_released()
{
  int selection = get_selected_server();
  if (selection >= 0)
  {
    Options::getInstance().removeFavorite(selection);
    list_favorites();
  }
}

void spritechat::Lobby::on_about_clicked()
{
  const bool hasApng = QImageReader::supportedImageFormats().contains("apng");
  QString msg = tr("<h2>Attorney Online %1</h2>"
                   "The courtroom drama simulator."
                   "<p><b>Source code:</b> "
                   "<a href='https://github.com/AttorneyOnline/AO2-Client'>"
                   "https://github.com/AttorneyOnline/AO2-Client</a>"
                   "<p><b>Major development:</b><br>"
                   "OmniTroid, stonedDiscord, longbyte1, scatterflower, Cerapter, "
                   "Crystalwarrior, Iamgoofball, in1tiate, Salanto"
                   "<p><b>Client development:</b><br>"
                   "Cents02, windrammer, skyedeving, TrickyLeifa, lambdcalculus"
                   "<p><b>QA testing:</b><br>"
                   "CaseyCazy, CedricDewitt, Chewable Tablets, CrazyJC, Fantos, "
                   "Fury McFlurry, Geck, Gin-Gi, Jamania, Minx, Pandae, "
                   "Robotic Overlord, Shadowlions (aka Shali), Sierra, SomeGuy, "
                   "Veritas, Wiso"
                   "<p><b>Translations:</b><br>"
                   "k-emiko (Русский), Pyraq (Polski), scatterflower (日本語), vintprox "
                   "(Русский), "
                   "windrammer (Español, Português)"
                   "<p><b>Special thanks:</b><br>"
                   "Wiso, dyviacat (2.10 release); "
                   "CrazyJC (2.8 release director) and MaximumVolty (2.8 release "
                   "promotion); "
                   "Remy, Hibiki, court-records.net (sprites); Qubrick (webAO); "
                   "Rue (website); Draxirch (UI design); "
                   "scatterflower and Salanto (akashi); "
                   "Lewdton and Argoneus (tsuserver); "
                   "Fiercy, Noevain, Cronnicossy, and FanatSors (AO1); "
                   "server hosts, game masters, case makers, content creators, "
                   "and the whole AO2 community!"
                   "<p>The Attorney Online networked visual novel project "
                   "is copyright (c) 2016-2022 Attorney Online developers. Open-source "
                   "licenses apply. All other assets are the property of their "
                   "respective owners."
                   "<p>Running on Qt version %2 with the BASS audio engine.<br>"
                   "APNG plugin loaded: %3"
                   "<p>Built on %4")
                    .arg(softwareVersion().toString())
                    .arg(QLatin1StringView(QT_VERSION_STR))
                    .arg(hasApng ? tr("Yes") : tr("No"))
                    .arg(QLatin1StringView(__DATE__));
  QMessageBox::about(this, tr("About"), msg);
}

// clicked on an item in the serverlist
void spritechat::Lobby::on_server_list_clicked(QTreeWidgetItem *p_item, int column)
{
  column = 0;
  ServerBookmark f_server;
  int n_server = p_item->text(column).toInt();

  if (n_server == last_index)
  {
    return;
  }
  last_index = n_server;

  if (n_server < 0)
  {
    return;
  }

  QList<ServerBookmark> f_server_list = master_gateway.serverList();

  if (n_server >= f_server_list.size())
  {
    return;
  }

  f_server = f_server_list.at(n_server);

  set_server_description(f_server.description);

  ui_server_description_text->moveCursor(QTextCursor::Start);
  ui_server_description_text->ensureCursorVisible();

  set_server_status(ServerStatus::Checking);

  server_info_gateway->requestInfo(f_server);
}

// doubleclicked on an item in the serverlist so we'll connect right away
void spritechat::Lobby::on_list_doubleclicked(QTreeWidgetItem *p_item, int column)
{
  Q_UNUSED(p_item)
  Q_UNUSED(column)
  if (m_server_status != ServerStatus::Online)
  {
    return;
  }
  on_connect_released();
}

void spritechat::Lobby::on_favorite_tree_clicked(QTreeWidgetItem *p_item, int column)
{
  column = 0;
  ServerBookmark f_server;
  int n_server = p_item->text(column).toInt();

  if (n_server == last_index)
  {
    return;
  }
  last_index = n_server;

  if (n_server < 0)
  {
    return;
  }

  ui_add_server_button->setEnabled(true);
  ui_edit_favorite_button->setEnabled(true);
  ui_remove_from_favorites_button->setEnabled(true);

  QList<ServerBookmark> f_server_list = Options::getInstance().favorites();

  if (n_server >= f_server_list.size())
  {
    return;
  }

  f_server = f_server_list.at(n_server);

  set_server_description(f_server.description);
  ui_server_description_text->moveCursor(QTextCursor::Start);
  ui_server_description_text->ensureCursorVisible();

  set_server_status(ServerStatus::Checking);

  server_info_gateway->requestInfo(f_server);
}

void spritechat::Lobby::on_server_search_edited(const QString &p_text)
{
  // Iterate through all QTreeWidgetItem items
  QTreeWidgetItemIterator it(ui_serverlist_tree);
  while (*it)
  {
    (*it)->setHidden(p_text != "");
    ++it;
  }

  if (p_text != "")
  {
    // Search in metadata
    QList<QTreeWidgetItem *> clist = ui_serverlist_tree->findItems(ui_serverlist_search->text(), Qt::MatchContains | Qt::MatchRecursive, 1);
    for (QTreeWidgetItem *item : std::as_const(clist))
    {
      if (item->parent() != nullptr) // So the category shows up too
      {
        item->parent()->setHidden(false);
      }
      item->setHidden(false);
    }
  }
}

void spritechat::Lobby::on_connect_released()
{
  const auto server = current_server();
  if (!server || m_server_status != ServerStatus::Online || net_manager.status() != NetworkManager::NotConnected)
  {
    return;
  }

  Q_EMIT connection_requested(server.value(), server_info_gateway->info());
}

void spritechat::Lobby::on_server_info_settled()
{
  const auto selected_server = current_server();
  const ServerBookmark queried_server = server_info_gateway->server();
  if (!selected_server || queried_server.address != selected_server->address || queried_server.port != selected_server->port)
  {
    return;
  }

  if (!server_info_gateway->isReachable())
  {
    set_server_status(ServerStatus::Offline);
  }
  else if (!server_info_gateway->isCompatible())
  {
    set_server_status(ServerStatus::Incompatible);
  }
  else
  {
    const auto info = server_info_gateway->info();
    set_player_count(info.playerCount, info.maxPlayers);
    if (!info.description.isEmpty())
    {
      set_server_description(info.description);
    }
    set_server_status(ServerStatus::Online);
  }

  update_connect_button();
}

void spritechat::Lobby::onReloadThemeRequested()
{
  // This is destructive to the active widget data.
  // Whatever, this is lobby. Nothing here is worth saving.
  delete centralWidget();

  reloadUi();
}

void spritechat::Lobby::onSettingsRequested()
{
  AOOptionsDialog options(ao_app);
  connect(&options, &AOOptionsDialog::reloadThemeRequest, this, &Lobby::onReloadThemeRequested);
  options.exec();

  ao_app->apply_master_options();
}

void spritechat::Lobby::list_servers()
{
  ui_serverlist_tree->setSortingEnabled(false);
  ui_serverlist_tree->clear();

  ui_serverlist_search->setText("");

  int i = 0;
  const QList<ServerBookmark> master_servers = master_gateway.serverList();
  for (const ServerBookmark &i_server : master_servers)
  {
    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui_serverlist_tree);
    treeItem->setData(0, Qt::DisplayRole, i);

    if (i_server.protocol == "tcp")
    {
      treeItem->setText(1, "(Legacy) " + i_server.name);
      treeItem->setBackground(0, Qt::darkRed);
      treeItem->setBackground(1, Qt::darkRed);

      QString tooltip = tr("Unable to connect to server. Server is missing WebSocket support.");
      treeItem->setToolTip(0, tooltip);
      treeItem->setToolTip(1, tooltip);
    }
    else
    {
      treeItem->setText(1, i_server.name);
    }

    i++;
  }
  ui_serverlist_tree->setSortingEnabled(true);
  ui_serverlist_tree->sortItems(0, Qt::SortOrder::AscendingOrder);
  ui_serverlist_tree->resizeColumnToContents(0);
}

void spritechat::Lobby::list_favorites()
{
  ui_favorites_tree->setSortingEnabled(false);
  ui_favorites_tree->clear();

  int i = 0;
  for (const ServerBookmark &i_server : Options::getInstance().favorites())
  {
    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui_favorites_tree);
    treeItem->setData(0, Qt::DisplayRole, i);

    if (i_server.protocol == "tcp")
    {
      treeItem->setText(1, "(Legacy) " + i_server.name);
      treeItem->setBackground(0, Qt::darkRed);
      treeItem->setBackground(1, Qt::darkRed);

      QString tooltip = tr("Unable to connect to server. Server is missing WebSocket support.");
      treeItem->setToolTip(0, tooltip);
      treeItem->setToolTip(1, tooltip);
    }
    else
    {
      treeItem->setText(1, i_server.name);
    }

    i++;
  }
  ui_favorites_tree->setSortingEnabled(true);
  ui_favorites_tree->sortItems(0, Qt::SortOrder::AscendingOrder);
  ui_favorites_tree->resizeColumnToContents(0);
}

void spritechat::Lobby::get_motd()
{
  master_gateway.requestMessageOfTheDay();
}

void spritechat::Lobby::show_message_of_the_day()
{
  QString document = master_gateway.messageOfTheDay();
  if (document.isEmpty())
  {
    document = tr("Couldn't get the message of the day.");
  }

  if (ui_motd_text)
  {
    ui_motd_text->setHtml(document);
  }
}

void spritechat::Lobby::check_for_updates()
{
  master_gateway.requestVersion();
}

void spritechat::Lobby::warn_about_outdated_client()
{
  QVersionNumber current_version = softwareVersion();
  QVersionNumber master_version = master_gateway.version();

  if (current_version >= master_version)
  {
    return;
  }

  ui_game_version_lbl->setText(tr("Version: %1 [OUTDATED]").arg(current_version.toString()));
  setWindowTitle(tr("[Your client is outdated]"));
  const QString download_url = convert_to_html(QStringLiteral("https://github.com/AttorneyOnline/AO2-Client/releases/latest"));
  const QString message = QString("Your client is outdated!<br>Your Version: %1<br>Current Version: %2<br>Download the latest version at<br>%3").arg(current_version.toString(), master_version.toString(), download_url);
  QMessageBox::warning(this, "Your client is outdated!", message);
}

void spritechat::Lobby::set_player_count(int players_online, int max_players)
{
  m_player_count = players_online;
  m_max_players = max_players;
}

void spritechat::Lobby::set_server_status(ServerStatus status)
{
  m_server_status = status;
  switch (status)
  {
  default:
  case ServerStatus::Offline:
    ui_server_player_count_lbl->setText(tr("Offline"));
    break;
  case ServerStatus::Checking:
    ui_server_player_count_lbl->setText(tr("Checking..."));
    break;
  case ServerStatus::Incompatible:
    ui_server_player_count_lbl->setText(tr("Incompatible server"));
    break;
  case ServerStatus::Online:
    ui_server_player_count_lbl->setText(tr("Online: %1/%2").arg(m_player_count).arg(m_max_players));
    break;
  }

  update_connect_button();
}

void spritechat::Lobby::update_connect_button()
{
  const auto selected_server = current_server();
  const bool server_ready = m_server_status == ServerStatus::Online;
  const bool network_idle = net_manager.status() == NetworkManager::NotConnected;

  ui_connect_button->setEnabled(selected_server && server_ready && network_idle);
}

void spritechat::Lobby::set_server_description(const QString &server_description)
{
  ui_server_description_text->clear();
  ui_server_description_text->insertHtml(convert_to_html(server_description));
}
