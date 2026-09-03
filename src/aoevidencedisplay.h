#pragma once

#include "animationlayer.h"
#include "aoapplication.h"
#include "aosfxplayer.h"
#include "game/game_defs.h"

#include <QDebug>
#include <QLabel>
#include <QPushButton>

namespace spritechat
{
class AOEvidenceDisplay : public QLabel
{
  Q_OBJECT

public:
  AOEvidenceDisplay(AOApplication *p_ao_app, QWidget *p_parent = nullptr);

  void show_evidence(theory::EvidenceId p_id, const QString &p_evidence_image, bool is_left_side, int p_volume);
  void reset();
  void combo_resize(int w, int h);
  void setLastEvidenceId(theory::EvidenceId f_id);

Q_SIGNALS:
  void show_evidence_details(theory::EvidenceId id);

private:
  AOApplication *ao_app;

  theory::EvidenceId m_last_evidence_id = theory::NoEvidenceId;
  AOSfxPlayer *m_sfx_player;

  InterfaceAnimationLayer *m_evidence_movie;
  QPushButton *ui_prompt_details;

private Q_SLOTS:
  void show_done();
  void icon_clicked();
};
} // namespace spritechat
