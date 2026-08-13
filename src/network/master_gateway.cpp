#include "master_gateway.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QLocale>

spritechat::MasterGateway::MasterGateway(QObject *parent)
    : QObject{parent}
{
  connect(&_client, &MasterClient::errorOccurred, this, &MasterGateway::errorOccurred);

  _heartbeatTimer.setInterval(std::chrono::minutes(5));
  connect(&_heartbeatTimer, &QTimer::timeout, this, &MasterGateway::postPlayerCount);
  _heartbeatTimer.start();
}

QUrl spritechat::MasterGateway::url() const
{
  return _url;
}

void spritechat::MasterGateway::setUrl(const QUrl &url)
{
  _url = url;
}

QString spritechat::MasterGateway::language() const
{
  return _language;
}

void spritechat::MasterGateway::setLanguage(const QString &language)
{
  _language = language;
}

QString spritechat::MasterGateway::acceptLanguage() const
{
  if (_language.trimmed().isEmpty())
  {
    return QLocale::system().name();
  }

  return _language;
}

bool spritechat::MasterGateway::telemetryEnabled() const
{
  return _telemetryEnabled;
}

void spritechat::MasterGateway::setTelemetryEnabled(bool enabled)
{
  _telemetryEnabled = enabled;
}

QList<spritechat::ServerBookmark> spritechat::MasterGateway::serverList() const
{
  return _serverList;
}

QString spritechat::MasterGateway::messageOfTheDay() const
{
  return _motd;
}

QVersionNumber spritechat::MasterGateway::version() const
{
  return _version;
}

QString spritechat::MasterGateway::privacyPolicy() const
{
  return _privacyPolicy;
}

void spritechat::MasterGateway::request(const QString &path, const MasterClient::Callback &callback)
{
  _client.request(_url, path, callback, acceptLanguage());
}

void spritechat::MasterGateway::requestServerList()
{
  static const QString path = QStringLiteral("/servers");
  request(path, [this](const QByteArray &payload) {
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(payload, &error);
    if (error.error)
    {
      notifyError(path, error.errorString());
      return;
    }

    if (!document.isArray())
    {
      notifyError(path, QStringLiteral("Invalid JSON document; expected array"));
      return;
    }

    QList<ServerBookmark> server_list;
    const QJsonArray entries = document.array();
    for (const QJsonValue &entryRef : entries)
    {
      if (!entryRef.isObject())
      {
        notifyError(path, QStringLiteral("Invalid JSON document; expected object"));
        return;
      }

      const QJsonObject entry = entryRef.toObject();

      ServerBookmark server;
      server.address = entry["ip"].toString();
      server.name = entry["name"].toString();
      server.description = entry["description"].toString(tr("No description provided."));
      if (entry.contains("wss_port"))
      {
        server.port = entry["wss_port"].toInt();
        server.protocol = QStringLiteral("wss");
      }
      else if (entry.contains("ws_port"))
      {
        server.port = entry["ws_port"].toInt();
        server.protocol = QStringLiteral("ws");
      }
      else
      {
        server.port = entry["port"].toInt();
        server.protocol = QStringLiteral("tcp");
      }

      if (server.port != 0)
      {
        server_list.append(server);
      }
    }

    _serverList = std::move(server_list);
    Q_EMIT serverListChanged();
  });
}

void spritechat::MasterGateway::requestMessageOfTheDay()
{
  static const QString path = QStringLiteral("/motd");
  request(path, [this](const QByteArray &payload) {
    _motd = QString::fromUtf8(payload);
    Q_EMIT messageOfTheDayChanged();
  });
}

void spritechat::MasterGateway::requestVersion()
{
  static const QString path = QStringLiteral("/version");
  request(path, [this](const QByteArray &payload) {
    _version = QVersionNumber::fromString(QString::fromUtf8(payload));
    Q_EMIT versionChanged();
  });
}

void spritechat::MasterGateway::requestPrivacyPolicy()
{
  static const QString path = QStringLiteral("/privacy");
  request(path, [this](const QByteArray &payload) {
    _privacyPolicy = QString::fromUtf8(payload);
    Q_EMIT privacyPolicyChanged();
  });
}

void spritechat::MasterGateway::postPlayerCount()
{
  if (!_telemetryEnabled)
  {
    return;
  }

  static const QString path = QStringLiteral("/playing");
  _client.post(_url, path, QByteArray());
}

void spritechat::MasterGateway::notifyError(const QString &path, const QString &what)
{
  Q_EMIT errorOccurred(QStringLiteral("API request %1 failed: %2").arg(path, what));
}
