#pragma once

#include "game/game_defs.h"

#include <QObject>

namespace spritechat
{
class Timer : public QObject
{
  Q_OBJECT

public:
  explicit Timer(theory::TimerId id, QObject *parent = nullptr);

  theory::TimerId id() const;

  theory::TimerState state() const;
  void setState(theory::TimerState state);

  qint64 remaining() const;
  void setRemaining(qint64 milliseconds);

  bool isVisible() const;
  void setVisible(bool visible);

  void reset();

Q_SIGNALS:
  void stateChanged(theory::TimerState state);
  void remainingChanged(qint64 milliseconds);
  void visibilityChanged(bool visible);

private:
  theory::TimerId _id;
  theory::TimerState _state = theory::TimerState::NotRunning;
  qint64 _remaining = 0;
  bool _visible = false;
};
} // namespace spritechat
