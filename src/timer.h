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

  qint64 remainingMs() const;
  void setRemainingMs(qint64 remainingMs);

  bool isVisible() const;
  void setVisible(bool visible);

  void reset();

Q_SIGNALS:
  void stateChanged(theory::TimerState state);
  void remainingMsChanged(qint64 remainingMs);
  void visibilityChanged(bool visible);

private:
  theory::TimerId _id;
  theory::TimerState _state = theory::TimerState::NotRunning;
  qint64 _remainingMs = 0;
  bool _visible = false;
};
} // namespace spritechat
