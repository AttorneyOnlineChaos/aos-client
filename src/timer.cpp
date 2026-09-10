#include "timer.h"

spritechat::Timer::Timer(theory::TimerId id, QObject *parent)
    : QObject{parent}
    , _id{id}
{}

theory::TimerId spritechat::Timer::id() const
{
  return _id;
}

theory::TimerState spritechat::Timer::state() const
{
  return _state;
}

void spritechat::Timer::setState(theory::TimerState state)
{
  if (_state == state)
  {
    return;
  }

  _state = state;
  Q_EMIT stateChanged(_state);
}

qint64 spritechat::Timer::remainingMs() const
{
  return _remainingMs;
}

void spritechat::Timer::setRemainingMs(qint64 remainingMs)
{
  const qint64 l_remaining = qMax<qint64>(0, remainingMs);
  if (_remainingMs == l_remaining)
  {
    return;
  }

  _remainingMs = l_remaining;
  Q_EMIT remainingMsChanged(_remainingMs);
}

bool spritechat::Timer::isVisible() const
{
  return _visible;
}

void spritechat::Timer::setVisible(bool visible)
{
  if (_visible == visible)
  {
    return;
  }

  _visible = visible;
  Q_EMIT visibilityChanged(_visible);
}

void spritechat::Timer::reset()
{
  setState(theory::TimerState::NotRunning);
  setRemainingMs(0);
  setVisible(false);
}
