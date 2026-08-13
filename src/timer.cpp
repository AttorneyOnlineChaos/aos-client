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

qint64 spritechat::Timer::remaining() const
{
  return _remaining;
}

void spritechat::Timer::setRemaining(qint64 milliseconds)
{
  const qint64 l_remaining = qMax<qint64>(0, milliseconds);
  if (_remaining == l_remaining)
  {
    return;
  }
  _remaining = l_remaining;
  Q_EMIT remainingChanged(_remaining);
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
  setRemaining(0);
  setVisible(false);
}
