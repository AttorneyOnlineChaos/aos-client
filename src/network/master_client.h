#pragma once

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

#include <functional>

namespace spritechat
{
class MasterClient : public QObject
{
  Q_OBJECT

public:
  explicit MasterClient(QObject *parent = nullptr);

  using Callback = std::function<void(const QByteArray &)>;
  void request(const QUrl &url, const QString &path, const Callback &callback, const QString &acceptLanguage = QString());
  void post(const QUrl &url, const QString &path, const QByteArray &payload);

Q_SIGNALS:
  void errorOccurred(const QString &error);

private:
  QNetworkAccessManager _manager;

  QNetworkRequest makeRequest(const QUrl &url, const QString &path, const QString &acceptLanguage) const;
  void notifyError(const QString &path, const QString &what);

private Q_SLOTS:
  void processPayload(const QString &path, const Callback &callback);
};
} // namespace spritechat
