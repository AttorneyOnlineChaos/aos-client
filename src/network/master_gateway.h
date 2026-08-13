#pragma once

#include "network/master_client.h"
#include "network/server_bookmark.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QVersionNumber>

namespace spritechat
{
class MasterGateway : public QObject
{
  Q_OBJECT

public:
  explicit MasterGateway(QObject *parent = nullptr);

  QUrl url() const;
  void setUrl(const QUrl &url);

  QString language() const;
  void setLanguage(const QString &language);

  bool telemetryEnabled() const;
  void setTelemetryEnabled(bool enabled);

  QList<ServerBookmark> serverList() const;
  QString messageOfTheDay() const;
  QVersionNumber version() const;
  QString privacyPolicy() const;

public Q_SLOTS:
  void requestServerList();
  void requestMessageOfTheDay();
  void requestVersion();
  void requestPrivacyPolicy();
  void postPlayerCount();

Q_SIGNALS:
  void errorOccurred(const QString &error);
  void serverListChanged();
  void messageOfTheDayChanged();
  void versionChanged();
  void privacyPolicyChanged();

private:
  MasterClient _client;
  QTimer _heartbeatTimer;

  QUrl _url;
  QString _language;
  bool _telemetryEnabled = true;

  QList<ServerBookmark> _serverList;
  QString _motd;
  QVersionNumber _version;
  QString _privacyPolicy;

  QString acceptLanguage() const;
  void request(const QString &path, const MasterClient::Callback &callback);
  void notifyError(const QString &path, const QString &what);
};
} // namespace spritechat
