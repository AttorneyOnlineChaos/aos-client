#pragma once

#include "aoapplication.h"

#include <QLabel>
#include <QPushButton>
#include <QShowEvent>

namespace spritechat
{
class AOEmoteButton : public QPushButton
{
  Q_OBJECT

public:
  AOEmoteButton(int id, int width, int height, AOApplication *ao_app, QWidget *parent = nullptr);

  int id();

  void setImage(const QString &character, bool selected);

  void setSelectedImage(const QString &p_image);

Q_SIGNALS:
  void emoteClicked(int p_id);

protected:
  void showEvent(QShowEvent *event) override;

private:
  AOApplication *ao_app;

  int m_id = 0;

  QString m_character;
  bool m_selected = false;
  bool m_loaded = false;

  QLabel *ui_selected = nullptr;

  void loadImage();
};
} // namespace spritechat
