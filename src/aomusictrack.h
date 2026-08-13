#pragma once

#include <QUrl>
#include <QtGlobal>

namespace spritechat
{
struct AOMusicTrack
{
  enum class LoopUnit
  {
    Frame,
    Second,
  };

  QUrl url;
  double loopStart = 0;
  double loopEnd = 0;
  LoopUnit loopUnit = LoopUnit::Frame;
};
} // namespace spritechat
