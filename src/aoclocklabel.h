#pragma once

#include "game/game_defs.h"

#include <QBasicTimer>
#include <QDateTime>
#include <QDebug>
#include <QLabel>
#include <QTimerEvent>

namespace spritechat
{
class AOClockLabel : public QLabel
{
  Q_OBJECT

public:
  AOClockLabel(QWidget *parent);

  void set(theory::TimerState state, qint64 remaining);
  void clear();

protected:
  void timerEvent(QTimerEvent *event) override;

private:
  QBasicTimer m_timer;
  QDateTime m_target_time;

  void refresh();
};
} // namespace spritechat
