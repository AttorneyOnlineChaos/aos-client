#pragma once

#include <QDebug>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTextCursor>

namespace spritechat
{
class AOTextArea : public QTextBrowser
{
  Q_OBJECT

public:
  AOTextArea(QWidget *parent = nullptr);
  AOTextArea(int maximumLogLenth, QWidget *parent = nullptr);

  void addMessage(const QString &name, QString message, const QString &nameColor, const QString &messageColor = QString(), const QString &timestamp = QString());

private:
  void auto_scroll(const QTextCursor &old_cursor, int scrollbar_value, bool is_scrolled_down);
};
} // namespace spritechat
