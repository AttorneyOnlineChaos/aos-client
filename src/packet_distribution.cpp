#include "aoapplication.h"

#include "area_registry.h"
#include "core/logging.h"
#include "courtroom.h"
#include "debug_functions.h"
#include "network_manager.h"
#include "options.h"
#include "player_registry.h"
#include "spritechat_log.h"

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

void spritechat::AOApplication::process(const theory::SessionGrantPacket &packet)
{
  m_tokens.insert(last_server.join_url(), packet.sessionToken);
}

void spritechat::AOApplication::process(const theory::WelcomePacket &packet)
{
  m_session_active = true;

  client_id = packet.clientId;

  if (w_courtroom->get_character_id() < 0)
  {
    w_courtroom->enter_char_select();
  }
  w_courtroom->show();

  destruct_lobby();
}

void spritechat::AOApplication::process(const theory::CharacterListPacket &packet)
{
  w_courtroom->clear_chars();
  for (const QString &character : packet.characters)
  {
    w_courtroom->append_char(character);
  }

  w_courtroom->character_loading_finished();
  w_courtroom->refresh_taken_chars();
}

void spritechat::AOApplication::process(const theory::MusicListPacket &packet)
{
  w_courtroom->set_music(packet.playlists);
  w_courtroom->list_music();
}

void spritechat::AOApplication::process(const theory::AreaListPacket &packet)
{
  m_area_registry.clear();

  theory::AreaId id = 0;
  for (const QString &area : packet.areas)
  {
    m_area_registry.add(id);
    m_area_registry.update(id, AreaInfo{.id = id, .name = area});
    ++id;
  }

  w_courtroom->list_areas();
}

void spritechat::AOApplication::process(const theory::CharacterAcceptedPacket &packet)
{
  if (packet.characterId < 0)
  {
    if (w_courtroom->get_character_id() >= 0)
    {
      w_courtroom->enter_char_select();
    }
    return;
  }

  w_courtroom->enter_courtroom();
  w_courtroom->set_courtroom_size();
  w_courtroom->update_character(packet.characterId);
}

void spritechat::AOApplication::process(const theory::BackgroundPacket &packet)
{
  if (packet.side)
  {
    w_courtroom->set_side(packet.side.value());
  }
  w_courtroom->set_background(packet.background, packet.display);
}

void spritechat::AOApplication::process(const theory::SetPositionPacket &packet)
{
  w_courtroom->set_side(packet.position.value_or(w_courtroom->default_side()));
}

void spritechat::AOApplication::process(const theory::AreaUpdatePacket &packet)
{
  const auto maybe_area = m_area_registry.area(packet.areaId);
  if (!maybe_area)
  {
    return;
  }
  AreaInfo area = maybe_area.value();

  std::optional<theory::JsonCodecError> error;
  switch (packet.property)
  {
  default:
    return;

  case theory::AreaUpdatePacket::Property::Status:
    error = theory::decodeJson(packet.data, area.status);
    break;

  case theory::AreaUpdatePacket::Property::Ownership:
    error = theory::decodeJson(packet.data, area.owners);
    break;

  case theory::AreaUpdatePacket::Property::Locked:
    error = theory::decodeJson(packet.data, area.lock);
    break;
  }

  if (error)
  {
    return;
  }

  m_area_registry.update(packet.areaId, area);
}

void spritechat::AOApplication::process(const theory::SubthemePacket &packet)
{
  subtheme = packet.subtheme;

  if (Options::getInstance().settingsSubTheme().toLower() != "server")
  {
    return;
  }

  Options::getInstance().setServerSubTheme(subtheme);
  w_courtroom->on_reload_theme_clicked();
}

spritechat::Timer *spritechat::AOApplication::timer(theory::TimerId id) const
{
  if (id < 0 || id >= m_timers.size())
  {
    return nullptr;
  }
  return m_timers.at(id);
}

void spritechat::AOApplication::process(const theory::TimerPacket &packet)
{
  Timer *l_timer = timer(packet.timerId);
  if (l_timer == nullptr)
  {
    return;
  }

  std::optional<theory::JsonCodecError> error;
  switch (packet.property)
  {
  default:
    return;

  case theory::TimerPacket::State:
    {
      const auto l_state = theory::decodeJson<theory::TimerState>(packet.data, error);
      if (error)
      {
        return;
      }
      l_timer->setState(l_state);
      break;
    }

  case theory::TimerPacket::Tick:
    {
      const qint64 l_remaining = theory::decodeJson<qint64>(packet.data, error);
      if (error)
      {
        return;
      }
      l_timer->setRemaining(l_remaining);
      break;
    }

  case theory::TimerPacket::Visibility:
    {
      const bool l_visible = theory::decodeJson<bool>(packet.data, error);
      if (error)
      {
        return;
      }
      l_timer->setVisible(l_visible);
      break;
    }
  }
}

void spritechat::AOApplication::process(const theory::IcMessagePacket &packet)
{
  w_courtroom->unpack_chatmessage(packet);
}

void spritechat::AOApplication::process(const theory::OocMessagePacket &packet)
{
  w_courtroom->append_server_chatmessage(packet.name, packet.message, QStringLiteral("0"));
}

void spritechat::AOApplication::process(const theory::ServerMessagePacket &packet)
{
  w_courtroom->append_server_chatmessage(tr("SERVER"), packet.message, QStringLiteral("1"));
  if (packet.level >= theory::ServerMessagePacket::Level::Notice)
  {
    call_notice(packet.message);
  }
}

void spritechat::AOApplication::process(const theory::MusicChangedPacket &packet)
{
  w_courtroom->handle_song(packet);
}

void spritechat::AOApplication::process(const theory::PenaltyPacket &packet)
{
  w_courtroom->set_hp_bar(packet.bar, packet.value);
}

void spritechat::AOApplication::process(const theory::SplashPacket &packet)
{
  w_courtroom->handle_wtce(packet);
}

void spritechat::AOApplication::process(const theory::EvidenceListPacket &packet)
{
  QList<theory::EvidenceItem> evidence = packet.items;
  w_courtroom->set_evidence_list(evidence);
}

void spritechat::AOApplication::process(const theory::PlayerRosterPacket &packet)
{
  switch (packet.action)
  {
  default:
    return;

  case theory::PlayerRosterPacket::Action::Add:
    m_player_registry.add(packet.clientId);
    break;

  case theory::PlayerRosterPacket::Action::Remove:
    m_player_registry.remove(packet.clientId);
    break;
  }
}

void spritechat::AOApplication::process(const theory::PlayerUpdatePacket &packet)
{
  const auto maybe_player = m_player_registry.player(packet.clientId);
  if (!maybe_player)
  {
    return;
  }
  PlayerInfo player = maybe_player.value();

  std::optional<theory::JsonCodecError> error;
  switch (packet.property)
  {
  default:
    return;

  case theory::PlayerUpdatePacket::Property::Name:
    error = theory::decodeJson(packet.data, player.name);
    break;

  case theory::PlayerUpdatePacket::Property::Character:
    error = theory::decodeJson(packet.data, player.character);
    break;

  case theory::PlayerUpdatePacket::Property::CharacterName:
    error = theory::decodeJson(packet.data, player.characterName);
    break;

  case theory::PlayerUpdatePacket::Property::AreaId:
    error = theory::decodeJson(packet.data, player.areaId);
    break;

  case theory::PlayerUpdatePacket::Property::Status:
    error = theory::decodeJson(packet.data, player.status);
    break;
  }

  if (error)
  {
    return;
  }

  m_player_registry.update(packet.clientId, player);
}

void spritechat::AOApplication::process(const theory::ModCallNoticePacket &packet)
{
  QString notice = tr("!!!MODCALL!!!\nArea: %1\nCaller: [%2]%3\n").arg(packet.area, QString::number(packet.callerClientId), packet.callerName);
  if (!packet.targetName.isEmpty())
  {
    notice.append(tr("Regarding: %1\n").arg(packet.targetName));
  }
  notice.append(tr("Reason: %1").arg(packet.reason));

  w_courtroom->mod_called(notice);
}

void spritechat::AOApplication::process(const theory::AuthStatePacket &packet)
{
  switch (packet.state)
  {
  default:
  case theory::AuthStatePacket::State::LoggedOut:
    w_courtroom->on_authentication_state_received(-1);
    break;

  case theory::AuthStatePacket::State::LoginFailed:
    w_courtroom->on_authentication_state_received(0);
    break;

  case theory::AuthStatePacket::State::LoggedIn:
    w_courtroom->on_authentication_state_received(1);
    break;
  }
}

void spritechat::AOApplication::process(const theory::ErrorPacket &packet)
{
  switch (packet.code)
  {
  default:
  case theory::ErrorPacket::ProtocolError:
    call_warning(tr("You have been dropped from the server.\n\nReason: %1").arg(packet.what));
    break;

  case theory::ErrorPacket::Banned:
    call_warning(tr("You have been banned from the server.\n\nReason: %1").arg(packet.what));
    break;

  case theory::ErrorPacket::ServerFull:
    call_warning(tr("The server is full.\n\nReason: %1").arg(packet.what));
    break;

  case theory::ErrorPacket::SessionTransfered:
    call_warning(tr("Your session has been resumed from another connection.\n\nReason: %1").arg(packet.what));
    break;
  }

  finish_session();
}
