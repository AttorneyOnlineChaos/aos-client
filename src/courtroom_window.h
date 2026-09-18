#pragma once

#include <QCloseEvent>
#include <QEvent>
#include <QMainWindow>
#include <QObject>
#include <QWidget>

namespace spritechat
{
class CourtroomWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit CourtroomWindow(QWidget *content);

Q_SIGNALS:
  void aboutToClose();

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;
  void closeEvent(QCloseEvent *event) override;

private:
  QWidget *_content = nullptr;
};
} // namespace spritechat
