#include "ao_line_edit.h"

spritechat::AOLineEdit::AOLineEdit(QWidget *parent)
    : QLineEdit{parent}
{}

int spritechat::AOLineEdit::capacity() const
{
  return _history.capacity();
}

void spritechat::AOLineEdit::setCapacity(int capacity)
{
  _history.setCapacity(capacity);
}

void spritechat::AOLineEdit::record()
{
  _history.record(text());
}

void spritechat::AOLineEdit::keyPressEvent(QKeyEvent *event)
{
  if (_history.capacity() == 0 || (event->modifiers() & ~Qt::KeypadModifier))
  {
    QLineEdit::keyPressEvent(event);
    return;
  }

  switch (event->key())
  {
  default:
    QLineEdit::keyPressEvent(event);
    break;
  case Qt::Key_Up:
    if (const auto message = _history.undo(text()))
    {
      setText(message.value());
      end(false);
    }

    event->accept();
    break;
  case Qt::Key_Down:
    if (const auto message = _history.redo())
    {
      setText(message.value());
      end(false);
    }

    event->accept();
    break;
  }
}
