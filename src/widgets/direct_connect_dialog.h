#pragma once

#include "network/server_bookmark.h"
#include "network/server_info_gateway.h"

#include "protocol/server_info.h"

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>

namespace spritechat
{
class DirectConnectDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DirectConnectDialog(QWidget *parent = nullptr);

Q_SIGNALS:
  void connection_requested(const ServerBookmark &server, const theory::ServerInfo &info);

private:
  static const QString UI_FILE_PATH;
  static const QRegularExpression SCHEME_PATTERN;

  ServerInfoGateway *m_info_gateway;

  QWidget *ui_widget;

  QLineEdit *ui_direct_hostname_edit;

  QLabel *ui_direct_connection_status_lbl;
  QPushButton *ui_direct_connect_button;
  QPushButton *ui_direct_cancel_button;

private Q_SLOTS:
  void onConnectPressed();
  void onServerInfoSettled();
};
} // namespace spritechat
