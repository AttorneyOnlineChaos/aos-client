#pragma once

#include "aoapplication.h"
#include "aoimage.h"
#include "game/game_defs.h"

#include <QEnterEvent>
#include <QFile>
#include <QPushButton>
#include <QString>
#include <QWidget>

namespace spritechat
{
class AOCharButton : public QPushButton
{
  Q_OBJECT

public:
  AOCharButton(AOApplication *ao_app, QWidget *parent);

  theory::CharacterId character() const;
  void setCharacter(const theory::CharacterId &character);

  void setTaken(bool enabled);

protected:
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;

private:
  AOApplication *ao_app;
  theory::CharacterId m_character;
  bool m_taken = false;
  AOImage *ui_taken;
  AOImage *ui_selector;
};
} // namespace spritechat
