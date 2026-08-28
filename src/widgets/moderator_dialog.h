#pragma once

#include "aoapplication.h"
#include "network/packet_transmitter.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSpinBox>
#include <QTextEdit>
#include <QWidget>

namespace spritechat
{
class ModeratorDialog : public QWidget
{
  Q_OBJECT

public:
  static const QString UI_FILE_PATH;

  explicit ModeratorDialog(theory::PlayerId playerId, bool ban, AOApplication *ao_app, theory::PacketTransmitter &transport, QWidget *parent = nullptr);
  virtual ~ModeratorDialog();

  theory::PlayerId playerId() const { return m_player_id; }

private:
  AOApplication *ao_app;
  theory::PacketTransmitter &m_transport;
  theory::PlayerId m_player_id;
  bool m_ban;

  QWidget *ui_widget;
  QComboBox *ui_action;
  QSpinBox *ui_duration_mm;
  QSpinBox *ui_duration_hh;
  QSpinBox *ui_duration_dd;
  QLabel *ui_duration_label;
  QCheckBox *ui_permanent;
  QTextEdit *ui_details;
  QDialogButtonBox *ui_button_box;

private Q_SLOTS:
  void onAcceptedClicked();
};
} // namespace spritechat
