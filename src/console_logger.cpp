#include "console_logger.h"

#include <QMetaType>

spritechat::ConsoleLogger::ConsoleLogger(QObject *parent)
    : QObject{parent}
{
  qRegisterMetaType<QtMsgType>();
}

void spritechat::ConsoleLogger::log(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
  Q_UNUSED(context)
  Q_EMIT messageLogged(type, message);
}
