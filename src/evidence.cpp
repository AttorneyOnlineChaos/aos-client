#include "courtroom.h"

#include "core/json_codec.h"
#include "core/logging.h"
#include "protocol/packets/evidence_packets.h"
#include "spritechat_defs.h"
#include "widgets/inventory_dialog.h"

#include <QDir>

void spritechat::Courtroom::initialize_evidence()
{
  ui_evidence_public = new EvidencePanel(EvidencePanel::Mode::Public, ao_app, evidence_registry, this);
  ui_evidence_public->setObjectName("ui_evidence");
  ui_evidence_private = new EvidencePanel(EvidencePanel::Mode::Private, ao_app, evidence_registry, this);
  ui_evidence_private->setObjectName("ui_evidence");
  ui_evidence_current = ui_evidence_public;

  evidence_refresh_timer = new QTimer(this);
  evidence_refresh_timer->setSingleShot(true);

  for (EvidencePanel *panel : {ui_evidence_public, ui_evidence_private})
  {
    connect(panel, &EvidencePanel::switchRequested, this, &Courtroom::switch_evidence_view);
    connect(panel, &EvidencePanel::presentingChanged, this, [this] { ui_ic_chat_message->setFocus(); });
  }

  connect(ui_evidence_private, &EvidencePanel::inventorySelected, this, &Courtroom::select_evidence_inventory);
  connect(ui_evidence_private, &EvidencePanel::addRequested, this, &Courtroom::add_evidence);
  connect(ui_evidence_private, &EvidencePanel::removeRequested, this, &Courtroom::remove_evidence);
  connect(ui_evidence_private, &EvidencePanel::snapshotRequested, this, &Courtroom::submit_evidence);
  connect(ui_evidence_private, &EvidencePanel::fileRequested, this, &Courtroom::open_evidence_file_dialog);

  connect(evidence_refresh_timer, &QTimer::timeout, this, &Courtroom::refresh_evidence_inventory);

  connect(&inventory_registry, &InventoryRegistry::added, this, &Courtroom::schedule_evidence_refresh);
  connect(&inventory_registry, &InventoryRegistry::removed, this, &Courtroom::schedule_evidence_refresh);
  connect(&inventory_registry, &InventoryRegistry::updated, this, &Courtroom::schedule_evidence_refresh);
  connect(&inventory_registry, &InventoryRegistry::cleared, this, &Courtroom::schedule_evidence_refresh);

  connect(&player_registry, &PlayerRegistry::added, this, &Courtroom::schedule_evidence_refresh);
  connect(&player_registry, &PlayerRegistry::removed, this, &Courtroom::schedule_evidence_refresh);
  connect(&player_registry, &PlayerRegistry::updated, this, &Courtroom::schedule_evidence_refresh);
  connect(&player_registry, &PlayerRegistry::cleared, this, &Courtroom::schedule_evidence_refresh);

  connect(&area_registry, &AreaRegistry::added, this, &Courtroom::schedule_evidence_refresh);
  connect(&area_registry, &AreaRegistry::removed, this, &Courtroom::schedule_evidence_refresh);
  connect(&area_registry, &AreaRegistry::updated, this, &Courtroom::schedule_evidence_refresh);
  connect(&area_registry, &AreaRegistry::cleared, this, &Courtroom::schedule_evidence_refresh);
}

void spritechat::Courtroom::refresh_evidence()
{
  set_size_and_pos(ui_evidence_button, "evidence_button");
  ui_evidence_button->setImage("evidence_button");
  ui_evidence_button->setToolTip(tr("Bring up the Evidence screen."));

  for (EvidencePanel *panel : {ui_evidence_public, ui_evidence_private})
  {
    set_font(panel->nameLine(), "", "evidence_name");
    set_font(panel->imageLine(), "", "evidence_image_name");
    set_font(panel->descriptionEdit(), "", "evidence_description");
    panel->applyTheme();
  }
}

bool spritechat::Courtroom::inventory_editable(theory::InventoryId inventory_id) const
{
  const auto inventory = inventory_registry.inventory(inventory_id);
  return inventory && inventory->permission == theory::InventoryPermission::Edit;
}

bool spritechat::Courtroom::current_inventory_editable() const
{
  return inventory_editable(current_inventory);
}

theory::InventoryId spritechat::Courtroom::personal_inventory() const
{
  const auto me = player_registry.player(ao_app->m_player_id);
  if (!me)
  {
    return theory::NoInventoryId;
  }
  return me->inventoryId;
}

std::optional<spritechat::PlayerInfo> spritechat::Courtroom::evidence_inventory_owner(theory::InventoryId inventory_id) const
{
  if (inventory_id == theory::NoInventoryId)
  {
    return std::nullopt;
  }
  const QList<PlayerInfo> owners = player_registry.playersIf([inventory_id](const PlayerInfo &player) { return player.inventoryId == inventory_id; });
  if (owners.isEmpty())
  {
    return std::nullopt;
  }
  return owners.first();
}

std::optional<spritechat::AreaInfo> spritechat::Courtroom::evidence_inventory_area(theory::InventoryId inventory_id) const
{
  if (inventory_id == theory::NoInventoryId)
  {
    return std::nullopt;
  }
  const QList<AreaInfo> areas = area_registry.areasIf([inventory_id](const AreaInfo &area) { return area.inventoryId == inventory_id; });
  if (areas.isEmpty())
  {
    return std::nullopt;
  }
  return areas.first();
}

QList<spritechat::InventoryInfo> spritechat::Courtroom::listed_evidence_inventories() const
{
  QList<InventoryInfo> listed = inventory_registry.inventoriesIf([](const InventoryInfo &inventory) { return inventory.permission == theory::InventoryPermission::Edit; });
  const theory::InventoryId personal = personal_inventory();
  std::stable_partition(listed.begin(), listed.end(), [personal](const InventoryInfo &inventory) { return personal != theory::NoInventoryId && inventory.id == personal; });
  return listed;
}

QString spritechat::Courtroom::evidence_player_label(const PlayerInfo &owner) const
{
  if (!owner.name.isEmpty())
  {
    return owner.name;
  }
  if (owner.character != theory::NoCharacterId)
  {
    return owner.character.toString();
  }
  return tr("Player %1").arg(owner.id);
}

QString spritechat::Courtroom::evidence_inventory_label(const InventoryInfo &inventory) const
{
  if (const auto area = evidence_inventory_area(inventory.id))
  {
    return tr("Area: %1").arg(area->displayName());
  }

  if (const auto owner = evidence_inventory_owner(inventory.id))
  {
    if (owner->id == ao_app->m_player_id)
    {
      return tr("Personal");
    }
    return evidence_player_label(owner.value());
  }

  return tr("Inventory %1").arg(inventory.id);
}

QList<spritechat::InventoryInfo> spritechat::Courtroom::public_evidence_inventories() const
{
  const auto me = player_registry.player(ao_app->m_player_id);
  if (!me)
  {
    return {};
  }

  const theory::AreaId my_area = me->areaId;
  QList<InventoryInfo> areas;
  QList<InventoryInfo> players;
  for (const InventoryInfo &inventory : inventory_registry.inventories())
  {
    if (const auto area = evidence_inventory_area(inventory.id))
    {
      if (area->id == my_area)
      {
        areas.append(inventory);
      }
    }
    else if (const auto owner = evidence_inventory_owner(inventory.id))
    {
      if (owner->areaId == my_area)
      {
        players.append(inventory);
      }
    }
  }
  return areas + players;
}

QString spritechat::Courtroom::public_evidence_inventory_label(const InventoryInfo &inventory) const
{
  if (const auto area = evidence_inventory_area(inventory.id))
  {
    return tr("Area: %1").arg(area->displayName());
  }

  if (const auto owner = evidence_inventory_owner(inventory.id))
  {
    return tr("Player: %1").arg(evidence_player_label(owner.value()));
  }

  return tr("Inventory %1").arg(inventory.id);
}

void spritechat::Courtroom::select_evidence_inventory(theory::InventoryId inventory_id)
{
  current_inventory = inventory_id;
}

void spritechat::Courtroom::switch_evidence_view()
{
  EvidencePanel *next = ui_evidence_private;
  if (ui_evidence_current == ui_evidence_private)
  {
    next = ui_evidence_public;
  }

  const bool shown = !ui_evidence_current->isHidden();
  ui_evidence_current->closeOverlay();
  ui_evidence_current->stopPresenting();
  ui_evidence_current->hide();

  ui_evidence_current = next;
  if (shown)
  {
    ui_evidence_current->show();
  }
}

void spritechat::Courtroom::show_evidence(theory::EvidenceId id)
{
  const auto item = evidence_registry.evidence(id);
  if (!item || !item->evidence.revealed)
  {
    return;
  }

  if (ui_evidence_current != ui_evidence_public)
  {
    switch_evidence_view();
  }
  ui_evidence_current->show();
  ui_evidence_public->showItem(id);
}

void spritechat::Courtroom::schedule_evidence_refresh()
{
  evidence_refresh_timer->start();
}

void spritechat::Courtroom::refresh_evidence_inventory()
{
  QList<EvidencePanel::InventoryChoice> choices;
  for (const InventoryInfo &inventory : listed_evidence_inventories())
  {
    choices.append(EvidencePanel::InventoryChoice{.id = inventory.id, .label = evidence_inventory_label(inventory)});
  }
  ui_evidence_private->setInventories(choices);

  QList<EvidencePanel::InventoryChoice> shared_choices;
  for (const InventoryInfo &inventory : public_evidence_inventories())
  {
    shared_choices.append(EvidencePanel::InventoryChoice{.id = inventory.id, .label = public_evidence_inventory_label(inventory)});
  }
  ui_evidence_public->setInventories(shared_choices);
}

void spritechat::Courtroom::add_evidence()
{
  if (!current_inventory_editable())
  {
    return;
  }

  theory::InventoryTransferPacket packet;
  packet.mode = theory::InventoryTransferPacket::Append;
  packet.inventoryId = current_inventory;
  packet.list.append(theory::Evidence{.name = "<name>", .description = "<description>", .image = "empty.png"});
  transport.shipPacket(packet);
}

void spritechat::Courtroom::remove_evidence(theory::EvidenceId id)
{
  const auto item = evidence_registry.evidence(id);
  if (!item || !inventory_editable(item->inventoryId))
  {
    return;
  }

  theory::EvidenceRecordPacket packet;
  packet.action = theory::EvidenceRecordPacket::Remove;
  packet.inventoryId = item->inventoryId;
  packet.evidenceId = item->id;
  transport.shipPacket(packet);
}

void spritechat::Courtroom::submit_evidence(theory::EvidenceId id, const theory::Evidence &evidence)
{
  const auto item = evidence_registry.evidence(id);
  if (!item || !inventory_editable(item->inventoryId))
  {
    return;
  }

  theory::EvidenceUpdatePacket packet;
  packet.evidenceId = id;
  packet.property = theory::EvidenceUpdatePacket::Snapshot;
  packet.data = theory::encodeJson(evidence);
  transport.shipPacket(packet);
}

void spritechat::Courtroom::open_evidence_file_dialog()
{
  const auto inventory = inventory_registry.inventory(current_inventory);
  if (!inventory || inventory->permission != theory::InventoryPermission::Edit)
  {
    return;
  }

  ui_evidence_private->closeOverlay();

  QString label = evidence_inventory_label(inventory.value());
  if (const auto area = evidence_inventory_area(inventory->id))
  {
    label = area->displayName();
    if (!area->name.isEmpty())
    {
      label = tr("%1 (Area %2)").arg(area->name).arg(area->id);
    }
  }

  QList<theory::Evidence> items;
  for (const EvidenceInfo &item : evidence_registry.evidenceIf([this](const EvidenceInfo &candidate) { return candidate.inventoryId == current_inventory; }))
  {
    items.append(item.evidence);
  }

  QDir(get_base_path()).mkpath("inventories");
  auto dialog = new InventoryDialog(get_base_path() + "inventories/", this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setInventory(label, items);
  connect(dialog, &InventoryDialog::transferRequested, this, &Courtroom::evidence_transfer);
  connect(dialog, &InventoryDialog::errorOccurred, this, [this](const QString &message) {
    zWarning(log::ic) << message;
    append_server_chatmessage(tr("CLIENT"), message, "1");
  });
  dialog->open();
}

void spritechat::Courtroom::evidence_transfer(theory::InventoryTransferPacket::Mode mode, const QList<theory::Evidence> &list)
{
  theory::InventoryTransferPacket packet;
  packet.mode = mode;
  packet.inventoryId = current_inventory;
  packet.list = list;
  transport.shipPacket(packet);
}
