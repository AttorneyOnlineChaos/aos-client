#include "aoapplication.h"

#include "core/logging.h"
#include "courtroom.h"
#include "debug_functions.h"
#include "hardware_functions.h"
#include "network_manager.h"
#include "options.h"
#include "protocol/packets/handshake_packets.h"
#include "protocol/protocol_info.h"
#include "spritechat_log.h"

#include <QDateTime>
#include <QMessageBox>
#include <QRegularExpression>

void spritechat::AOApplication::shipPacket(const theory::Packet &packet)
{
  net_manager->shipPacket(packet);
}

void spritechat::AOApplication::register_packet_routes()
{
  m_router.registerRoute<theory::SessionGrantPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::WelcomePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::CharacterListPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::MusicListPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::AreaListPacket>(&AOApplication::process, this);

  m_router.registerRoute<theory::CharacterAcceptedPacket>(&AOApplication::process, this);

  m_router.registerRoute<theory::BackgroundPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::SetPositionPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::AreaUpdatePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::SubthemePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::TimerPacket>(&AOApplication::process, this);

  m_router.registerRoute<theory::IcMessagePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::OocMessagePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::MusicChangedPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::ServerMessagePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::PenaltyPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::SplashPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::EvidenceListPacket>(&AOApplication::process, this);

  m_router.registerRoute<theory::PlayerRosterPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::PlayerUpdatePacket>(&AOApplication::process, this);

  m_router.registerRoute<theory::ModCallNoticePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::AuthStatePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::ErrorPacket>(&AOApplication::process, this);
}

void spritechat::AOApplication::handle_network_status(NetworkManager::Status status)
{
  switch (status)
  {
  default:
  case NetworkManager::Connecting:
    break;

  case NetworkManager::Connected:
    start_session();
    break;

  case NetworkManager::NotConnected:
    stop_session();
    break;
  }
}

void spritechat::AOApplication::handle_network_error(const theory::CargoError &error)
{
  call_warning(tr("Connection error.\n\nDetails: %1").arg(error.toString()));
}

void spritechat::AOApplication::connect_to_server(const ServerBookmark &server, const theory::ServerInfo &info)
{
  if (net_manager->status() != NetworkManager::NotConnected)
  {
    zWarning(log::network) << "already connected to a server";
    return;
  }

  m_server = server;
  m_server_info = info;

  m_server_data.set_server_software(info.softwareName);
  m_server_data.set_asset_url(info.assetUrl);

  server_name = info.name.isEmpty() ? server.name : info.name;
  window_title = server_name;

  QString server_address = QString("%1:%2").arg(server.address, QString::number(server.port));
  QString server_name_stripped = server_name;
  static QRegularExpression illegal_filename_chars("[\\\\/:*?\"<>|\']");
  log_filename = QDateTime::currentDateTime().toUTC().toString("'logs/" + server_name_stripped.remove(illegal_filename_chars) + "/'yyyy-MM-dd hh-mm-ss t'.log'");
  write_to_file("Joined server " + server_name_stripped + " hosted on address " + server_address + " on " + QDateTime::currentDateTime().toUTC().toString(), log_filename, true);

  m_session_active = false;

  net_manager->connectToServer(server);
}

void spritechat::AOApplication::reconnect_to_server()
{
  net_manager->connectToServer(m_server);
}

void spritechat::AOApplication::start_session()
{
  zInfo(log::network) << "established connection to server.";

  reset_server_instance();

  if (!m_session_active)
  {
    construct_courtroom();
  }

  theory::HelloPacket packet;
  packet.hdid = get_hdid();
  packet.protocolVersion = theory::protocolVersion();
  shipPacket(packet);

  theory::SessionClaimPacket claim;
  const QUrl server_url = m_server.join_url();
  if (m_tokens.contains(server_url))
  {
    claim.sessionToken = m_tokens.value(server_url);
  }
  shipPacket(claim);
}

void spritechat::AOApplication::stop_session()
{
  Options::getInstance().setServerSubTheme(QString());

  if (m_session_active)
  {
    w_courtroom->setEnabled(false);

    const bool reconnect = QMessageBox::question(nullptr,
                                                 tr("Server Disconnected"),
                                                 tr("Connection to the server has been lost. "
                                                    "Do you want to reconnect?"),
                                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

    if (reconnect)
    {
      reconnect_to_server();
      return;
    }
  }
  m_session_active = false;

  construct_lobby();
  destruct_courtroom();
}

void spritechat::AOApplication::drop_session()
{
  m_session_active = false;
  m_tokens.remove(m_server.join_url());
}

void spritechat::AOApplication::process(const theory::SessionGrantPacket &packet)
{
  m_tokens.insert(m_server.join_url(), packet.sessionToken);
  m_recovered_session = packet.result == theory::SessionGrantPacket::Recovered;
}

void spritechat::AOApplication::process(const theory::WelcomePacket &packet)
{
  m_session_active = true;

  client_id = packet.clientId;

  if (m_recovered_session)
  {
    w_courtroom->enter_char_select();
  }
  w_courtroom->setEnabled(true);
  w_courtroom->show();

  destruct_lobby();
}

void spritechat::AOApplication::process_pending_packets()
{
  while (net_manager->hasPendingPacket())
  {
    theory::PacketPointer packet = net_manager->nextPacket();
    if (!packet)
    {
      return;
    }

    if (!m_router.route(*packet))
    {
      zWarning(log::protocol) << QStringLiteral("failed to route packet: %1").arg(packet->header());
    }
  }
}
