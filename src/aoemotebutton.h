#pragma once

#include "aoapplication.h"

#include <QLabel>
#include <QPushButton>

namespace spritechat
{
class AOEmoteButton : public QPushButton
{
  Q_OBJECT

public:
  AOEmoteButton(int id, int width, int height, AOApplication *ao_app, QWidget *parent = nullptr);

  int id();

  void setImage(const QString &character, int emoteId, bool enabled);

  void setSelectedImage(const QString &p_image);

Q_SIGNALS:
  void emoteClicked(int p_id);

private:
  AOApplication *ao_app;

  int m_id = 0;

  QLabel *ui_selected = nullptr;
};
} // namespace spritechat
