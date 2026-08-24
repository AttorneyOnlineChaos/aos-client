#include "moderator_dialog.h"

#include "ao_widget_lookup.h"
#include "aoapplication.h"
#include "options.h"
#include "protocol/packets/moderation_packets.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QUiLoader>
#include <QVBoxLayout>

#include <chrono>

const QString spritechat::ModeratorDialog::UI_FILE_PATH = "moderator_action_dialog.ui";

spritechat::ModeratorDialog::ModeratorDialog(int clientId, bool ban, AOApplication *ao_app, theory::PacketTransmitter &transport, QWidget *parent)
    : QWidget{parent}
    , ao_app(ao_app)
    , m_transport(transport)
    , m_client_id(clientId)
    , m_ban(ban)
{
  QFile file(Options::getInstance().getUIAsset(UI_FILE_PATH));
  if (!file.open(QFile::ReadOnly))
  {
    qFatal("Unable to open file %s", qPrintable(file.fileName()));
    return;
  }

  setWindowIcon(QIcon(":/data/logo-client.png"));
  QUiLoader loader;
  ui_widget = loader.load(&file, this);
  auto layout = new QVBoxLayout(this);
  layout->addWidget(ui_widget);

  AOWidgetLookup l_ui{this};

  l_ui.find(ui_action, "action");
  l_ui.find(ui_duration_mm, "duration_mm");
  l_ui.find(ui_duration_hh, "duration_hh");
  l_ui.find(ui_duration_dd, "duration_dd");
  l_ui.find(ui_duration_label, "duration_label");
  l_ui.find(ui_permanent, "permanent");
  l_ui.find(ui_details, "details");
  l_ui.find(ui_button_box, "button_box");

  if (m_ban)
  {
    ui_action->addItem(tr("Ban"));
  }
  else
  {
    ui_action->addItem(tr("Kick"));
  }

  ui_duration_mm->setVisible(m_ban);
  ui_duration_hh->setVisible(m_ban);
  ui_duration_dd->setVisible(m_ban);
  ui_duration_label->setVisible(m_ban);
  ui_permanent->setVisible(m_ban);

  connect(ui_button_box, &QDialogButtonBox::accepted, this, &ModeratorDialog::onAcceptedClicked);
  connect(ui_button_box, &QDialogButtonBox::rejected, this, &ModeratorDialog::close);
}

spritechat::ModeratorDialog::~ModeratorDialog()
{}

void spritechat::ModeratorDialog::onAcceptedClicked()
{
  QString reason = ui_details->toPlainText();
  if (reason.isEmpty())
  {
    if (QMessageBox::question(this, tr("Confirmation"), tr("Are you sure you want to confirm without a reason?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    {
      return;
    }
  }

  bool permanent = ui_permanent->isChecked();
  if (permanent)
  {
    if (QMessageBox::question(this, tr("Confirmation"), tr("Are you sure you want to ban permanently?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    {
      return;
    }
  }

  theory::ModActionPacket packet;
  packet.targetClientId = m_client_id;
  packet.reason = reason;
  if (m_ban)
  {
    packet.action = theory::ModActionPacket::Action::Ban;
    if (permanent)
    {
      packet.durationSeconds = -1;
    }
    else
    {
      qint64 duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::days(ui_duration_dd->value())).count();
      duration = duration + std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(ui_duration_hh->value())).count();
      duration = duration + std::chrono::duration_cast<std::chrono::seconds>(std::chrono::minutes(ui_duration_mm->value())).count();
      packet.durationSeconds = duration;
    }
  }
  else
  {
    packet.action = theory::ModActionPacket::Action::Kick;
  }

  m_transport.shipPacket(packet);

  close();
}
