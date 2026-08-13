#pragma once

#include <QString>

namespace spritechat
{
bool file_exists(const QString &file_path);
bool dir_exists(const QString &file_path);
bool exists(const QString &p_path);

QString get_app_path();
QString get_base_path();
} // namespace spritechat
