#include "aoapplication.h"

#include "core/logging.h"
#include "courtroom.h"
#include "debug_functions.h"
#include "lobby.h"
#include "network_manager.h"
#include "options.h"
#include "protocol/packets/handshake_packets.h"
#include "protocol/protocol_info.h"
#include "spritechat_defs.h"

#include <QDateTime>
#include <QLayout>
#include <QMessageBox>
#include <QRegularExpression>

void spritechat::AOApplication::shipPacket(const theory::Packet &packet)
{
  net_manager->shipPacket(packet);
}

void spritechat::AOApplication::register_packet_routes()
{
  m_router.registerRoute<theory::BadgeSelectionPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::BadgePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::SessionGrantPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::ServerSettingsPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::WelcomePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::CharacterListPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::MusicListPacket>(&AOApplication::process, this);

  m_router.registerRoute<theory::CharacterAcceptedPacket>(&AOApplication::process, this);

  m_router.registerRoute<theory::BackgroundPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::SetPositionPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::AreaRecordPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::AreaUpdatePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::SubthemePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::TimerPacket>(&AOApplication::process, this);

  m_router.registerRoute<theory::IcMessagePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::OocMessagePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::MusicChangedPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::ServerMessagePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::PenaltyPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::SplashPacket>(&AOApplication::process, this);

  m_router.registerRoute<theory::PlayerRecordPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::PlayerUpdatePacket>(&AOApplication::process, this);

  m_router.registerRoute<theory::InventoryRecordPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::InventoryUpdatePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::EvidenceRecordPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::EvidenceUpdatePacket>(&AOApplication::process, this);

  m_router.registerRoute<theory::ModCallNoticePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::AuthStatePacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::GameErrorPacket>(&AOApplication::process, this);
  m_router.registerRoute<theory::ErrorPacket>(&AOApplication::process, this);
}

void spritechat::AOApplication::handle_network_status(NetworkManager::Status status)
{
  switch (status)
  {
  default:
  case NetworkManager::Connecting:
  case NetworkManager::NotConnected:
    break;

  case NetworkManager::Connected:
    start_session();
    break;
  }
}

void spritechat::AOApplication::handle_network_error(const theory::CargoError &error)
{
  call_warning(tr("Connection error.\n\nDetails: %1").arg(error.toString()));
}

void spritechat::AOApplication::connect_to_server(const ServerBookmark &server)
{
  if (net_manager->status() != NetworkManager::NotConnected)
  {
    zWarning(log::network) << "already connected to a server";
    return;
  }

  m_server = server;

  server_name = server.name;
  window_title = server_name;

  QString server_address = QString("%1:%2").arg(server.address, QString::number(server.port));
  QString server_name_stripped = server_name;
  static QRegularExpression illegal_filename_chars("[\\\\/:*?\"<>|\']");
  log_filename = QDateTime::currentDateTime().toUTC().toString("'logs/" + server_name_stripped.remove(illegal_filename_chars) + "/'yyyy-MM-dd hh-mm-ss t'.log'");
  write_to_file("Joined server " + server_name_stripped + " hosted on address " + server_address + " on " + QDateTime::currentDateTime().toUTC().toString(), log_filename, true);

  m_session_active = false;

  net_manager->setAllowInsecureTls(Options::getInstance().allowInsecureTls());
  net_manager->connectToServer(server);
}

void spritechat::AOApplication::reconnect_to_server()
{
  net_manager->setAllowInsecureTls(Options::getInstance().allowInsecureTls());
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

  openSignIn();

  theory::HelloPacket packet;
  packet.protocolVersion = theory::protocolVersion();
  shipPacket(packet);

  theory::SessionClaimPacket claim;
  const QUrl server_url = m_server.join_url();
  if (m_tokens.contains(server_url))
  {
    claim.sessionToken = m_tokens.value(server_url);
  }

  claim.userToken = _userTokens.token(server_url);
  shipPacket(claim);
}

void spritechat::AOApplication::stop_session(theory::CargoSocket::Closure closure)
{
  closeSignIn();

  Options::getInstance().setServerSubTheme(QString());

  if (m_session_active && !theory::CargoSocket::isGracefulClosure(closure))
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
  closeSignIn();
  m_session_active = false;
  m_tokens.remove(m_server.join_url());
}

void spritechat::AOApplication::leaveServer()
{
  drop_session();
  construct_lobby();
  destruct_courtroom();
  net_manager->disconnectFromServer();
}

void spritechat::AOApplication::openSignIn()
{
  _badgeClient = theory::makeUnique<theory::BadgeClientEngine>(_badgeFactory);
  connect(_badgeClient.get(), &theory::BadgeClientEngine::badgeSelected, this, &AOApplication::shipBadgeSelection);
  connect(_badgeClient.get(), &theory::BadgeClientEngine::badgeSelected, this, &AOApplication::showSignInWidget);
  connect(_badgeClient.get(), &theory::BadgeClientEngine::responseReady, this, &AOApplication::sendBadgeResponse);
  connect(_badgeClient.get(), &theory::BadgeClientEngine::responseReady, this, &AOApplication::showSignInWidget);
  connect(_badgeClient.get(), &theory::BadgeClientEngine::errorOccurred, this, &AOApplication::abortSignIn, Qt::QueuedConnection);
  connect(_badgeClient.get(), &theory::BadgeClientEngine::cancelled, this, &AOApplication::leaveSignIn, Qt::QueuedConnection);
  connect(_badgeClient.get(), &theory::BadgeClientEngine::interactionRequired, this, &AOApplication::showSignInWidget);
}

void spritechat::AOApplication::closeSignIn()
{
  delete _badgeBackdrop;
  _badgeClient.reset();
}

void spritechat::AOApplication::shipBadgeSelection(const QString &badgeId)
{
  theory::BadgeSelectPacket select;
  select.badgeId = badgeId;
  shipPacket(select);
}

void spritechat::AOApplication::sendBadgeResponse(const QString &badgeId, const QJsonObject &responseData)
{
  theory::BadgePacket badge;
  badge.badgeId = badgeId;
  badge.payload = responseData;
  shipPacket(badge);
}

void spritechat::AOApplication::abortSignIn(const QString &message)
{
  call_warning(message);
  net_manager->disconnectFromServer();
}

void spritechat::AOApplication::leaveSignIn()
{
  net_manager->disconnectFromServer();
}

void spritechat::AOApplication::showSignInWidget()
{
  if (!_badgeBackdrop)
  {
    QWidget *window = _courtroomWindow;
    if (w_lobby)
    {
      window = w_lobby->centralWidget();
    }

    _badgeBackdrop = new theory::Backdrop{window};
    _badgeBackdrop->setObjectName(QStringLiteral("badge_backdrop"));
    _badgeBackdrop->container()->setObjectName(QStringLiteral("badge_container"));
  }

  if (_badgeWidget)
  {
    _badgeWidget->hide();
    _badgeWidget->deleteLater();
  }

  QWidget *container = _badgeBackdrop->container();
  _badgeWidget = _badgeClient->createWidget(container);
  container->layout()->addWidget(_badgeWidget);
  _badgeBackdrop->show();
}

void spritechat::AOApplication::process(const theory::BadgeSelectionPacket &packet)
{
  if (!_badgeClient)
  {
    zWarning(log::protocol) << "badge selection outside authentication window";
    return;
  }

  _badgeClient->processSelection(packet.badgeIds);
}

void spritechat::AOApplication::process(const theory::BadgePacket &packet)
{
  if (!_badgeClient)
  {
    zWarning(log::protocol) << "badge challenge outside authentication window";
    return;
  }

  _badgeClient->processChallenge(packet.badgeId, packet.payload);
}

void spritechat::AOApplication::process(const theory::SessionGrantPacket &packet)
{
  closeSignIn();
  m_tokens.insert(m_server.join_url(), packet.sessionToken);
  _userTokens.setToken(m_server.join_url(), packet.userToken);
  if (const auto error = _userTokens.save())
  {
    zWarning(log::main) << QStringLiteral("user tokens: %1").arg(error->toString());
  }

  m_recovered_session = packet.result == theory::SessionGrantPacket::Recovered;
}

void spritechat::AOApplication::process(const theory::ServerSettingsPacket &packet)
{
  server_name = packet.settings.name.isEmpty() ? m_server.name : packet.settings.name;
  window_title = server_name;
  _courtroomWindow->setWindowTitle(window_title);

  m_server_settings.setSettings(packet.settings);
}

void spritechat::AOApplication::process(const theory::WelcomePacket &packet)
{
  m_session_active = true;

  m_player_id = packet.playerId;

  if (!m_recovered_session)
  {
    w_courtroom->enter_char_select();
  }

  w_courtroom->setEnabled(true);
  _courtroomWindow->show();

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

    if (const auto error = packet->verify())
    {
      net_manager->disconnectFromServer();
      call_warning(tr("Protocol error.\n\nDetails: %1: %2").arg(packet->header(), error->toString()));
      return;
    }

    if (!m_router.route(*packet))
    {
      zWarning(log::protocol) << QStringLiteral("failed to route packet: %1").arg(packet->header());
    }
  }
}
