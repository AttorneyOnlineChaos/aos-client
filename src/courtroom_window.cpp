#include "courtroom_window.h"

#include <QIcon>
#include <QString>

spritechat::CourtroomWindow::CourtroomWindow(QWidget *content)
    : QMainWindow{}
    , _content{content}
{
  setObjectName(QStringLiteral("courtroom_window"));
  setWindowIcon(QIcon{QStringLiteral(":/data/logo-client.png")});
  setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

  setCentralWidget(_content);
  _content->installEventFilter(this);
  setFixedSize(_content->size());
}

bool spritechat::CourtroomWindow::eventFilter(QObject *watched, QEvent *event)
{
  if (watched == _content && event->type() == QEvent::Resize)
  {
    setFixedSize(_content->size());
  }

  return QMainWindow::eventFilter(watched, event);
}

void spritechat::CourtroomWindow::closeEvent(QCloseEvent *event)
{
  Q_EMIT aboutToClose();

  QMainWindow::closeEvent(event);
}
