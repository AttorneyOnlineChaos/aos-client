#pragma once

#include "game/evidence.h"
#include "game/game_defs.h"

#include <QList>
#include <QMap>
#include <QString>

#include <optional>

namespace spritechat
{
class BackgroundPosition
{
public:
  QString background;
  QString desk;
  std::optional<int> origin;
};

struct pos_size_type
{
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

enum EMOTE_MOD_TYPE
{
  IDLE = 0,
  PREANIM = 1,
  ZOOM = 5,
  PREANIM_ZOOM = 6,
};

enum DESK_MOD_TYPE
{
  DESK_HIDE = 0,
  DESK_SHOW,
  DESK_EMOTE_ONLY,
  DESK_PRE_ONLY,
  DESK_EMOTE_ONLY_EX,
  DESK_PRE_ONLY_EX,
  //"EX" for "expanded"
  // dumb, i know, but throw the first stone if you have a better idea
};

enum MUSIC_EFFECT
{
  FADE_IN = 1,
  FADE_OUT = 2,
  SYNC_POS = 4,
  NO_REPEAT = 8
};

enum RESIZE_MODE
{
  AUTO_RESIZE_MODE,
  PIXEL_RESIZE_MODE,
  SMOOTH_RESIZE_MODE,
};

struct PlayerInfo
{
  theory::PlayerId id = theory::NoPlayerId;
  QString name;
  theory::CharacterId character;
  std::optional<QString> characterName;
  theory::AreaId areaId = 0;
  theory::PlayerStatus status = theory::PlayerStatus::Online;
  theory::InventoryId inventoryId = theory::NoInventoryId;
};

struct AreaInfo
{
  theory::AreaId id = theory::NoAreaId;
  QString name;
  theory::AreaStatus status = theory::AreaStatus::Idle;
  QList<theory::PlayerId> owners;
  theory::AreaLockStatus lock = theory::AreaLockStatus::Unlocked;
  theory::InventoryId inventoryId = theory::NoInventoryId;

  QString displayName() const;
};

struct InventoryInfo
{
  theory::InventoryId id = theory::NoInventoryId;
  theory::InventoryPermission permission = theory::InventoryPermission::NoPermission;
};

struct EvidenceInfo
{
  theory::EvidenceId id = theory::NoEvidenceId;
  theory::InventoryId inventoryId = theory::NoInventoryId;
  theory::Evidence evidence;
};

} // namespace spritechat
