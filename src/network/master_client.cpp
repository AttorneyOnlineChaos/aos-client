#include "master_client.h"

#include "spritechat_info.h"

#include <QNetworkReply>

#include <functional>

spritechat::MasterClient::MasterClient(QObject *parent)
    : QObject{parent}
{}

QNetworkRequest spritechat::MasterClient::makeRequest(const QUrl &url, const QString &path, const QString &acceptLanguage) const
{
  QUrl destination(url);
  destination.setPath(path);

  QNetworkRequest request(destination);
  request.setRawHeader("User-Agent", softwareUserAgent().toUtf8());
  if (!acceptLanguage.isEmpty())
  {
    request.setRawHeader("Accept-Language", acceptLanguage.toUtf8());
  }

  return request;
}

void spritechat::MasterClient::request(const QUrl &url, const QString &path, const Callback &callback, const QString &acceptLanguage)
{
  if (!url.isValid())
  {
    notifyError(path, QStringLiteral("Invalid master server url."));
    return;
  }

  QNetworkReply *reply = _manager.get(makeRequest(url, path, acceptLanguage));
  connect(reply, &QNetworkReply::finished, this, std::bind(&MasterClient::processPayload, this, path, callback));
}

void spritechat::MasterClient::post(const QUrl &url, const QString &path, const QByteArray &payload)
{
  if (!url.isValid())
  {
    notifyError(path, QStringLiteral("Invalid master server url."));
    return;
  }

  QNetworkReply *reply = _manager.post(makeRequest(url, path, QString()), payload);
  connect(reply, &QNetworkReply::finished, this, [this, path, reply] {
    if (reply->error() != QNetworkReply::NoError)
    {
      notifyError(path, reply->errorString());
    }
    reply->deleteLater();
  });
}

void spritechat::MasterClient::notifyError(const QString &path, const QString &what)
{
  Q_EMIT errorOccurred(QStringLiteral("API request %1 failed: %2").arg(path, what));
}

void spritechat::MasterClient::processPayload(const QString &path, const Callback &callback)
{
  QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError)
  {
    notifyError(path, reply->errorString());
    return;
  }

  int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (status != 200)
  {
    notifyError(path, QStringLiteral("http status %1").arg(status));
    return;
  }

  std::invoke(callback, reply->readAll());
}
