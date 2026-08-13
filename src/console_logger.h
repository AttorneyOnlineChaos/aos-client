#pragma once

#include "core/logger.h"

#include <QObject>
#include <QString>
#include <QtLogging>

namespace spritechat
{
class ConsoleLogger : public QObject, public theory::Logger
{
  Q_OBJECT

public:
  explicit ConsoleLogger(QObject *parent = nullptr);

  void log(QtMsgType type, const QMessageLogContext &context, const QString &message) override;

Q_SIGNALS:
  void messageLogged(QtMsgType type, const QString &message);
};
} // namespace spritechat
