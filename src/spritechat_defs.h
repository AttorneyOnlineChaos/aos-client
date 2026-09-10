#pragma once

#include "game/chat_markup.h"

#include <QLoggingCategory>

namespace spritechat
{
struct ChatMarkupEntry
{
  int index = 0;
  theory::ChatMarkup markup;

  bool isValid() const;
};

namespace log
{
Q_DECLARE_LOGGING_CATEGORY(main)
Q_DECLARE_LOGGING_CATEGORY(network)
Q_DECLARE_LOGGING_CATEGORY(protocol)
Q_DECLARE_LOGGING_CATEGORY(viewport)
Q_DECLARE_LOGGING_CATEGORY(effect)
Q_DECLARE_LOGGING_CATEGORY(ic)
Q_DECLARE_LOGGING_CATEGORY(character)
Q_DECLARE_LOGGING_CATEGORY(asset)
Q_DECLARE_LOGGING_CATEGORY(ui)
Q_DECLARE_LOGGING_CATEGORY(audio)
Q_DECLARE_LOGGING_CATEGORY(plugin)
} // namespace log
} // namespace spritechat
