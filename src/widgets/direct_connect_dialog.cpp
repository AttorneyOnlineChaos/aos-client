#include "direct_connect_dialog.h"

#include "ao_widget_lookup.h"
#include "core/logging.h"
#include "debug_functions.h"
#include "options.h"
#include "spritechat_log.h"

#include <QFile>
#include <QStringBuilder>
#include <QUiLoader>
#include <QVBoxLayout>

const QString spritechat::DirectConnectDialog::UI_FILE_PATH = "direct_connect_dialog.ui";
const QRegularExpression spritechat::DirectConnectDialog::SCHEME_PATTERN{"^\\w+://.+$"};

spritechat::DirectConnectDialog::DirectConnectDialog(QWidget *parent)
    : QDialog(parent)
{
  setWindowIcon(QIcon(":/data/logo-client.png"));
  QUiLoader l_loader(this);
  QFile l_uiFile(Options::getInstance().getUIAsset(UI_FILE_PATH));

  if (!l_uiFile.open(QFile::ReadOnly))
  {
    zCritical(log::ui) << "Unable to open file " << l_uiFile.fileName();
    return;
  }
  ui_widget = l_loader.load(&l_uiFile, this);

  auto l_layout = new QVBoxLayout(this);
  l_layout->addWidget(ui_widget);

  m_info_gateway = new ServerInfoGateway(this);
  connect(m_info_gateway, &ServerInfoGateway::infoSettled, this, &DirectConnectDialog::onServerInfoSettled);

  AOWidgetLookup l_ui{this};

  l_ui.find(ui_direct_hostname_edit, "direct_hostname_edit");

  l_ui.find(ui_direct_connection_status_lbl, "direct_connection_status_lbl");

  l_ui.find(ui_direct_connect_button, "direct_connect_button");
  connect(ui_direct_connect_button, &QPushButton::pressed, this, &DirectConnectDialog::onConnectPressed);
  l_ui.find(ui_direct_cancel_button, "direct_cancel_button");
  connect(ui_direct_cancel_button, &QPushButton::pressed, this, &DirectConnectDialog::close);
}

void spritechat::DirectConnectDialog::onConnectPressed()
{
  QString l_hostname = ui_direct_hostname_edit->text();
  if (!SCHEME_PATTERN.match(l_hostname).hasMatch())
  {
    l_hostname = "ws://" % l_hostname;
  }

  QUrl l_url(l_hostname);
  if (!l_url.isValid())
  {
    call_warning(tr("Invalid URL."));
    return;
  }

  if (l_url.scheme() != "ws" && l_url.scheme() != "wss")
  {
    call_warning(tr("Invalid URL scheme. Only ws:// and wss:// are supported."));
    return;
  }

  if (l_url.port() == -1)
  {
    call_warning(tr("Invalid server port."));
    return;
  }
  ServerBookmark l_server;
  l_server.address = l_url.host();
  l_server.port = l_url.port();
  l_server.protocol = l_url.scheme();
  l_server.name = "Direct Connection";

  m_info_gateway->requestInfo(l_server);
  ui_direct_connect_button->setEnabled(false);
  ui_direct_connection_status_lbl->setText("Connecting...");
  ui_direct_connection_status_lbl->setStyleSheet("color : rgb(0,64,156)");
}

void spritechat::DirectConnectDialog::onServerInfoSettled()
{
  if (!m_info_gateway->isReachable())
  {
    ui_direct_connect_button->setEnabled(true);
    ui_direct_connection_status_lbl->setText("Could not reach server!");
    ui_direct_connection_status_lbl->setStyleSheet("color: rgb(255,0,0)");
    return;
  }

  if (!m_info_gateway->isCompatible())
  {
    ui_direct_connect_button->setEnabled(true);
    ui_direct_connection_status_lbl->setText("Incompatible server!");
    ui_direct_connection_status_lbl->setStyleSheet("color: rgb(255,0,0)");
    return;
  }

  ui_direct_connection_status_lbl->setText("Connected!");
  ui_direct_connection_status_lbl->setStyleSheet("color: rgb(0,128,0)");
  Q_EMIT connection_requested(m_info_gateway->server(), m_info_gateway->info());
  close();
}
