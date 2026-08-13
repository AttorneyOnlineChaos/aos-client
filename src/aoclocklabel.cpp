#include "aoclocklabel.h"

spritechat::AOClockLabel::AOClockLabel(QWidget *parent)
    : QLabel(parent)
{}

void spritechat::AOClockLabel::set(theory::TimerState state, qint64 remaining)
{
  m_target_time = QDateTime::currentDateTime().addMSecs(qMax(qint64(0), remaining));

  if (state == theory::TimerState::Running)
  {
    m_timer.start(1000 / 60, this);
  }
  else
  {
    m_timer.stop();
  }

  this->refresh();
}

void spritechat::AOClockLabel::clear()
{
  m_timer.stop();
  m_target_time = QDateTime();
  this->setText("00:00:00.000");
}

void spritechat::AOClockLabel::timerEvent(QTimerEvent *event)
{
  if (event->timerId() != m_timer.timerId())
  {
    QWidget::timerEvent(event);
    return;
  }

  this->refresh();

  if (QDateTime::currentDateTime() >= m_target_time)
  {
    m_timer.stop();
  }
}

void spritechat::AOClockLabel::refresh()
{
  if (!m_target_time.isValid())
  {
    this->setText("00:00:00.000");
    return;
  }

  const qint64 ms_left = QDateTime::currentDateTime().msecsTo(m_target_time);
  if (ms_left <= 0)
  {
    this->setText("00:00:00.000");
    return;
  }

  QTime timeleft = QTime(0, 0).addMSecs(ms_left % (1000 * 3600 * 24));
  this->setText(timeleft.toString("hh:mm:ss.zzz"));
}
