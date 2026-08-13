#include "playerlistwidget.h"

#include "aoapplication.h"
#include "debug_functions.h"
#include "moderation_functions.h"
#include "player_registry.h"
#include "protocol/packets/moderation_packets.h"
#include "widgets/moderator_dialog.h"

#include <QListWidgetItem>
#include <QMenu>

#include <optional>

spritechat::PlayerListWidget::PlayerListWidget(AOApplication *ao_app, PlayerRegistry &player_registry, QWidget *parent)
    : QListWidget(parent)
    , ao_app(ao_app)
    , m_registry(player_registry)
{
  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(&m_registry, &PlayerRegistry::added, this, &PlayerListWidget::addPlayer);
  connect(&m_registry, &PlayerRegistry::removed, this, &PlayerListWidget::removePlayer);
  connect(&m_registry, &PlayerRegistry::updated, this, &PlayerListWidget::refreshPlayer);
  connect(&m_registry, &PlayerRegistry::cleared, this, &PlayerListWidget::clearPlayers);

  connect(this, &PlayerListWidget::customContextMenuRequested, this, &PlayerListWidget::onCustomContextMenuRequested);
}

void spritechat::PlayerListWidget::reloadPlayers()
{
  for (auto it = m_item_map.constBegin(); it != m_item_map.constEnd(); ++it)
  {
    refreshPlayer(it.key());
  }
}

void spritechat::PlayerListWidget::setArea(theory::AreaId area)
{
  m_area = area;
  filterPlayerList();
}

void spritechat::PlayerListWidget::setAuthenticated(bool f_state)
{
  m_is_authenticated = f_state;
  filterPlayerList();
}

void spritechat::PlayerListWidget::onCustomContextMenuRequested(const QPoint &pos)
{
  QListWidgetItem *item = itemAt(pos);
  if (item == nullptr)
  {
    return;
  }
  const theory::ClientId id = item->data(Qt::UserRole).toInt();
  QString name = item->text();

  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);

  QAction *report = menu->addAction("Report Player");
  connect(report, &QAction::triggered, this, [this, id, name] {
    auto maybe_reason = call_moderator_support(name);
    if (maybe_reason)
    {
      theory::ModCallPacket packet;
      packet.reason = maybe_reason.value();
      packet.targetClientId = id;
      ao_app->shipPacket(packet);
    }
  });

  if (m_is_authenticated)
  {
    QAction *kick = menu->addAction("Kick");
    connect(kick, &QAction::triggered, this, [this, id, name] {
      m_dialog = theory::makeUnique<ModeratorDialog>(id, false, ao_app);
      m_dialog->setWindowTitle(tr("Kick %1").arg(name));
      m_dialog->show();
    });

    QAction *ban = menu->addAction("Ban");
    connect(ban, &QAction::triggered, this, [this, id, name] {
      m_dialog = theory::makeUnique<ModeratorDialog>(id, true, ao_app);
      m_dialog->setWindowTitle(tr("Ban %1").arg(name));
      m_dialog->show();
    });
  }

  menu->popup(mapToGlobal(pos));
}

void spritechat::PlayerListWidget::addPlayer(theory::ClientId id)
{
  QListWidgetItem *item = new QListWidgetItem(this);
  item->setData(Qt::UserRole, id);
  m_item_map.insert(id, item);
  refreshPlayer(id);
}

void spritechat::PlayerListWidget::removePlayer(theory::ClientId id)
{
  if (m_dialog && m_dialog->clientId() == id)
  {
    m_dialog.reset();
    call_warning("Closed Moderation Dialog : User left the server.");
  }

  delete takeItem(row(m_item_map.take(id)));
}

void spritechat::PlayerListWidget::refreshPlayer(theory::ClientId id)
{
  QListWidgetItem *item = m_item_map.value(id);
  if (!item)
  {
    return;
  }

  const auto maybe_player = m_registry.player(id);
  if (!maybe_player)
  {
    return;
  }
  const PlayerInfo player = maybe_player.value();

  item->setText(formatLabel(player));

  if (player.character.isEmpty())
  {
    item->setToolTip(QString());
    item->setIcon(QIcon());
  }
  else
  {
    QString tooltip = player.character;
    if (player.characterName)
    {
      tooltip = QObject::tr("%1 aka %2").arg(player.character, player.characterName.value());
    }
    item->setToolTip(tooltip);
    item->setIcon(QIcon(ao_app->get_image_suffix(ao_app->get_character_path(player.character, "char_icon"), true)));
  }

  filterPlayerList();
}

void spritechat::PlayerListWidget::clearPlayers()
{
  clear();
  m_item_map.clear();
}

void spritechat::PlayerListWidget::filterPlayerList()
{
  for (auto it = m_item_map.constBegin(); it != m_item_map.constEnd(); ++it)
  {
    const auto player = m_registry.player(it.key());
    const theory::AreaId player_area = player ? player->areaId : theory::NoAreaId;
    QListWidgetItem *item = it.value();
    item->setHidden(player_area != m_area && !m_is_authenticated);
  }
}

QString spritechat::PlayerListWidget::formatLabel(const PlayerInfo &data)
{
  auto statusLabel = [](theory::PlayerStatus status) -> QString {
    switch (status)
    {
    default:
    case theory::PlayerStatus::Online:
      return QString();
    case theory::PlayerStatus::Away:
      return QObject::tr("Away");
    }
  };

  QString format = Options::getInstance().playerlistFormatString();
  return format.replace("{id}", QString::number(data.id)).replace("{character}", data.character).replace("{displayname}", data.characterName.value_or(QStringLiteral("No Data"))).replace("{username}", data.name).replace("{status}", statusLabel(data.status)).simplified();
}
