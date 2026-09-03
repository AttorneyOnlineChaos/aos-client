#pragma once

#include "aoapplication.h"
#include "aoimage.h"
#include "game/game_defs.h"

#include <QDebug>
#include <QEnterEvent>
#include <QPushButton>
#include <QString>

namespace spritechat
{
class AOEvidenceButton : public QPushButton
{
  Q_OBJECT

public:
  AOEvidenceButton(theory::EvidenceId id, int width, int height, AOApplication *ao_app, QWidget *parent = nullptr);

  void setImage(const QString &fileName);

  void setThemeImage(const QString &fileName);

  void setSelected(bool enabled);
  void setRevealed(bool revealed);

Q_SIGNALS:
  void evidenceClicked(theory::EvidenceId id);
  void evidenceDoubleClicked(theory::EvidenceId id);

  void mouseoverUpdated(theory::EvidenceId id, bool state);

protected:
  void enterEvent(QEnterEvent *e) override;
  void leaveEvent(QEvent *e) override;

  void mouseDoubleClickEvent(QMouseEvent *e) override;

private:
  AOApplication *ao_app;

  theory::EvidenceId m_id = theory::NoEvidenceId;

  AOImage *ui_hidden;
  AOImage *ui_selected;
  AOImage *ui_selector;

private Q_SLOTS:
  void on_clicked();
};
} // namespace spritechat
