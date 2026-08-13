#pragma once

#include <QMessageBox>
#include <QString>

namespace spritechat
{
void call_notice(const QString &message);
void call_warning(const QString &message);
void call_error(const QString &message);
} // namespace spritechat
