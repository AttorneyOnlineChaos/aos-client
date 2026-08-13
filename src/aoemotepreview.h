#pragma once

#include "animationlayer.h"
#include <QWidget>

namespace spritechat
{
class AOEmotePreview : public QWidget
{
  Q_OBJECT

public:
  AOEmotePreview(AOApplication *ao_app, QWidget *parent = nullptr);

  void display(const QString &character, const QString &emote, CharacterAnimationLayer::EmoteType emoteType, bool flipped = false, int xOffset = 0, int yOffset = 0);

  void updateViewportGeometry();

protected:
  void resizeEvent(QResizeEvent *event);

private:
  AOApplication *ao_app;

  QString m_character;
  QString m_emote;

  QWidget *ui_viewport;
  BackgroundAnimationLayer *ui_vp_background;
  SplashAnimationLayer *ui_vp_speedlines;
  CharacterAnimationLayer *ui_vp_player_char;
  BackgroundAnimationLayer *ui_vp_desk;
  QLabel *ui_size_label;
};
} // namespace spritechat
