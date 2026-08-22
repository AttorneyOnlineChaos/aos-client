#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "aoapplication.h"
#include "network/master_gateway.h"
#include "network/server_info_gateway.h"
#include "network_manager.h"

#include "protocol/server_info.h"

#include <QMainWindow>

#include <optional>

namespace spritechat
{
class Lobby : public QMainWindow
{
  Q_OBJECT

public:
  Lobby(AOApplication *p_ao_app, NetworkManager &network, MasterGateway &master);

  void set_player_count(int players_online, int max_players);
  void set_server_description(const QString &server_description);
  void list_servers();
  int get_selected_server() const;
  std::optional<ServerBookmark> current_server() const;

Q_SIGNALS:
  void connection_requested(const ServerBookmark &server, const theory::ServerInfo &info);

private:
  AOApplication *ao_app;
  NetworkManager &net_manager;
  MasterGateway &master_gateway;
  ServerInfoGateway *server_info_gateway;

  const QString DEFAULT_UI = "lobby.ui";

  enum class ServerStatus
  {
    Offline,
    Checking,
    Incompatible,
    Online,
  };

  void list_favorites();
  void get_motd();
  void check_for_updates();
  void reset_selection();
  void set_server_status(ServerStatus status);
  void update_connect_button();

  int last_index = -1;

  ServerStatus m_server_status = ServerStatus::Offline;
  int m_player_count = 0;
  int m_max_players = 0;

  enum TabPage
  {
    SERVER,
    FAVORITES,
    DEMOS
  };

  // UI-file Lobby

  // Top Row
  QLabel *ui_game_version_lbl;
  QPushButton *ui_settings_button;
  QPushButton *ui_about_button;

  // Server, Favs and Demo lists
  QTabWidget *ui_connections_tabview;

  QTreeWidget *ui_serverlist_tree;
  QLineEdit *ui_serverlist_search;

  QTreeWidget *ui_favorites_tree;

  QPushButton *ui_add_to_favorite_button;
  QPushButton *ui_add_server_button;
  QPushButton *ui_remove_from_favorites_button;
  QPushButton *ui_edit_favorite_button;
  QPushButton *ui_direct_connect_button;
  QPushButton *ui_refresh_button;

  // Serverinfo / MOTD Horizontal Row
  QPointer<QTextBrowser> ui_motd_text;

  QLabel *ui_server_player_count_lbl;
  QTextBrowser *ui_server_description_text;
  QPushButton *ui_connect_button;

  // Optional Widget
  QTextBrowser *ui_game_changelog_text;

  void loadUI();
  void reloadUi();

  TabPage current_page = SERVER;

private Q_SLOTS:
  void on_tab_changed(int index);
  void on_refresh_released();
  void on_direct_connect_released();
  void on_add_to_fav_released();
  void on_add_server_to_fave_released();
  void on_edit_favorite_released();
  void on_remove_from_fav_released();
  void on_about_clicked();
  void on_server_list_clicked(QTreeWidgetItem *p_item, int column);
  void on_list_doubleclicked(QTreeWidgetItem *p_item, int column);
  void on_favorite_tree_clicked(QTreeWidgetItem *p_item, int column);
  void on_server_search_edited(const QString &p_text);
  void on_connect_released();
  void on_server_info_settled();
  void show_message_of_the_day();
  void warn_about_outdated_client();
  void onReloadThemeRequested(); // Oh boy.
  void onSettingsRequested();
};
} // namespace spritechat
